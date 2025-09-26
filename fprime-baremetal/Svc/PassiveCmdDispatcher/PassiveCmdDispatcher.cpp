// ======================================================================
// \title  PassiveCmdDispatcher.cpp
// \author root
// \brief  cpp file for PassiveCmdDispatcher component implementation class
// ======================================================================

#include <Fw/Cmd/CmdPacket.hpp>
#include <fprime-baremetal/Svc/PassiveCmdDispatcher/PassiveCmdDispatcher.hpp>

#include <limits>
#include <new>

namespace Baremetal {

// Check the CMD_DISPATCHER_DISPATCH_TABLE_SIZE and CMD_DISPATCHER_SEQUENCER_TABLE_SIZE constants for overflow
static_assert(CMD_DISPATCHER_DISPATCH_TABLE_SIZE <= std::numeric_limits<FwOpcodeType>::max(),
              "Opcode table limited to opcode range");
static_assert(CMD_DISPATCHER_SEQUENCER_TABLE_SIZE <= std::numeric_limits<U32>::max(),
              "Sequencer table limited to range of U32");

// Indicates that an entry in the dispatch table or sequence tracker is unused
constexpr FwOpcodeType OPCODE_UNUSED = std::numeric_limits<FwOpcodeType>::max();

// ----------------------------------------------------------------------
// Component construction and destruction
// ----------------------------------------------------------------------

PassiveCmdDispatcher::PassiveCmdDispatcher(const char* const compName)
    : PassiveCmdDispatcherComponentBase(compName), m_seq(0) {}

PassiveCmdDispatcher::CmdTables::CmdTables() {
    for (auto i = 0; i < CMD_DISPATCHER_DISPATCH_TABLE_SIZE; i++) {
        this->m_entryTable[i].opcode = OPCODE_UNUSED;
    }
    for (auto i = 0; i < CMD_DISPATCHER_SEQUENCER_TABLE_SIZE; i++) {
        this->m_sequenceTracker[i].opcode = OPCODE_UNUSED;
    }
}

PassiveCmdDispatcher::~PassiveCmdDispatcher() {
    if (this->m_cmdTables != nullptr) {
        // First destruct the tables
        this->m_cmdTables->~CmdTables();
        // Then deallocate the memory
        if (this->m_allocator != nullptr) {
            void* memory = this->m_cmdTables;
            this->m_allocator->deallocate(this->m_memId, memory);
        }
    }
}

void PassiveCmdDispatcher::setup(FwEnumStoreType memId, Fw::MemAllocator& allocator) {
    FW_ASSERT(this->m_cmdTables == nullptr);
    this->m_allocator = &allocator;
    this->m_memId = memId;

    // Allocate a chunk of memory for the command tables
    FwSizeType expected_size = sizeof(CmdTables);
    FwSizeType allocated_size = expected_size;
    bool recoverable = false;
    void* memory = allocator.allocate(memId, allocated_size, recoverable);
    FW_ASSERT((memory != nullptr) && (allocated_size == expected_size), allocated_size);
    this->m_cmdTables = new (memory) CmdTables();
}

// ----------------------------------------------------------------------
// Handler implementations for typed input ports
// ----------------------------------------------------------------------

void PassiveCmdDispatcher::compCmdReg_handler(FwIndexType portNum, FwOpcodeType opCode) {
    FW_ASSERT(this->m_cmdTables != nullptr);
    // This opcode is reserved for internal use
    FW_ASSERT(opCode != OPCODE_UNUSED, portNum);

    // Search for an empty slot
    bool slotFound = false;
    for (auto slot = 0; slot < CMD_DISPATCHER_DISPATCH_TABLE_SIZE; slot++) {
        if ((this->m_cmdTables->m_entryTable[slot].opcode == OPCODE_UNUSED) && (!slotFound)) {
            // Empty slot found
            this->m_cmdTables->m_entryTable[slot].opcode = opCode;
            this->m_cmdTables->m_entryTable[slot].port = portNum;
            slotFound = true;
        } else if ((this->m_cmdTables->m_entryTable[slot].opcode == opCode) &&
                   (this->m_cmdTables->m_entryTable[slot].port == portNum) && (!slotFound)) {
            // Found a slot where the opcode was already registered
            slotFound = true;
        } else if (this->m_cmdTables->m_entryTable[slot].opcode != OPCODE_UNUSED) {
            // Ensure that there are no duplicates
            FW_ASSERT(this->m_cmdTables->m_entryTable[slot].opcode != opCode, static_cast<FwAssertArgType>(opCode));
        }
    }
    FW_ASSERT(slotFound, static_cast<FwAssertArgType>(opCode));
}

void PassiveCmdDispatcher::compCmdStat_handler(FwIndexType portNum,
                                               FwOpcodeType opCode,
                                               U32 cmdSeq,
                                               const Fw::CmdResponse& response) {
    FW_ASSERT(this->m_cmdTables != nullptr);
    // This opcode is reserved for internal use
    FW_ASSERT(opCode != OPCODE_UNUSED, portNum);

    // Check the command response and log success/failure
    if (response.e == Fw::CmdResponse::OK) {
        this->log_COMMAND_OpCodeCompleted(opCode);
    } else {
        FW_ASSERT(response.e != Fw::CmdResponse::OK);
        this->log_COMMAND_OpCodeError(opCode, response);
    }

    // Search for the command source
    FwIndexType portToCall = -1;
    U32 context = 0;
    for (auto pending = 0; pending < CMD_DISPATCHER_SEQUENCER_TABLE_SIZE; pending++) {
        auto entry = &this->m_cmdTables->m_sequenceTracker[pending];
        if ((entry->opcode != OPCODE_UNUSED) && (entry->seq == cmdSeq)) {
            portToCall = entry->callerPort;
            context = entry->context;
            FW_ASSERT(opCode == entry->opcode);
            FW_ASSERT(portToCall < this->getNum_seqCmdStatus_OutputPorts());
            // Free up the sequence tracker entry
            entry->opcode = OPCODE_UNUSED;
            break;
        }
    }
    // If found, call the port to report the command status
    if ((portToCall != -1) && this->isConnected_seqCmdStatus_OutputPort(portToCall)) {
        // NOTE: seqCmdStatus port forwards three arguments (opCode, cmdSeq, response) but the
        // cmdSeq value has no meaning for the calling sequencer; instead, the context value is
        // forwarded to allow the caller to utilize it if needed.
        this->seqCmdStatus_out(portToCall, opCode, context, response);
    }
}

void PassiveCmdDispatcher::seqCmdBuff_handler(FwIndexType portNum, Fw::ComBuffer& data, U32 context) {
    FW_ASSERT(this->m_cmdTables != nullptr);

    // Deserialize the command packet
    Fw::CmdPacket cmdPkt;
    Fw::SerializeStatus stat = cmdPkt.deserialize(data);
    if (stat != Fw::FW_SERIALIZE_OK) {
        Fw::DeserialStatus serErr = static_cast<Fw::DeserialStatus::t>(stat);
        this->log_WARNING_HI_MalformedCommand(serErr);
        if (this->isConnected_seqCmdStatus_OutputPort(portNum)) {
            this->seqCmdStatus_out(portNum, cmdPkt.getOpCode(), context, Fw::CmdResponse::VALIDATION_ERROR);
        }
        return;
    }
    FwOpcodeType opcode = cmdPkt.getOpCode();

    // Search for the opcode in the dispatch table
    DispatchEntry* entry = nullptr;
    // Ignore OPCODE_UNUSED, reserved for internal use
    if (opcode != OPCODE_UNUSED) {
        for (auto slot = 0; slot < CMD_DISPATCHER_DISPATCH_TABLE_SIZE; slot++) {
            if (this->m_cmdTables->m_entryTable[slot].opcode == opcode) {
                entry = &this->m_cmdTables->m_entryTable[slot];
            }
        }
    }
    if ((entry != nullptr) && this->isConnected_compCmdSend_OutputPort(entry->port)) {
        // Register the command in the command tracker only if the response port is connected
        if (this->isConnected_seqCmdStatus_OutputPort(portNum)) {
            bool pendingFound = false;
            for (U32 pending = 0; pending < CMD_DISPATCHER_SEQUENCER_TABLE_SIZE; pending++) {
                SequenceTracker* trackerEntry = &this->m_cmdTables->m_sequenceTracker[pending];
                if (trackerEntry->opcode == OPCODE_UNUSED) {
                    pendingFound = true;
                    trackerEntry->opcode = opcode;
                    trackerEntry->callerPort = portNum;
                    trackerEntry->seq = this->m_seq;
                    trackerEntry->context = context;
                    break;
                }
            }
            // If no slot was found to track the command, quit
            if (!pendingFound) {
                this->log_WARNING_HI_TooManyCommands(opcode);
                if (this->isConnected_seqCmdStatus_OutputPort(portNum)) {
                    this->seqCmdStatus_out(portNum, opcode, context, Fw::CmdResponse::EXECUTION_ERROR);
                }
                return;
            }
        }

        // Pass arguments to the argument buffer and log the dispatched command
        this->compCmdSend_out(entry->port, opcode, this->m_seq, cmdPkt.getArgBuffer());
        this->log_COMMAND_OpCodeDispatched(opcode, entry->port);
    } else {
        // Opcode could not be found in the dispatch table, fail the command
        this->log_WARNING_HI_InvalidCommand(opcode);
        if (this->isConnected_seqCmdStatus_OutputPort(portNum)) {
            this->seqCmdStatus_out(portNum, opcode, context, Fw::CmdResponse::INVALID_OPCODE);
        }
    }

    // Increment sequence number
    this->m_seq++;
}

// ----------------------------------------------------------------------
// Handler implementations for commands
// ----------------------------------------------------------------------

void PassiveCmdDispatcher::CMD_NO_OP_cmdHandler(FwOpcodeType opCode, U32 cmdSeq) {
    FW_ASSERT(this->m_cmdTables != nullptr);

    this->log_ACTIVITY_HI_NoOpReceived();
    this->cmdResponse_out(opCode, cmdSeq, Fw::CmdResponse::OK);
}

void PassiveCmdDispatcher::CMD_CLEAR_TRACKING_cmdHandler(FwOpcodeType opCode, U32 cmdSeq) {
    FW_ASSERT(this->m_cmdTables != nullptr);

    // Clear the sequence tracking table
    for (FwOpcodeType entry = 0; entry < CMD_DISPATCHER_SEQUENCER_TABLE_SIZE; entry++) {
        this->m_cmdTables->m_sequenceTracker[entry].opcode = OPCODE_UNUSED;
    }
    this->cmdResponse_out(opCode, cmdSeq, Fw::CmdResponse::OK);
}

}  // namespace Baremetal
