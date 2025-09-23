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
    : PassiveCmdDispatcherComponentBase(compName), m_seq(0), m_setupDone(false) {}

PassiveCmdDispatcher::~PassiveCmdDispatcher() {
    // Memory is allocated in a contiguous chunk for both the dispatch table and the sequence
    // tracker so just check if the dispatch table was allocated
    if (this->m_entryTable != nullptr) {
        FW_ASSERT(this->m_sequenceTracker != nullptr);
        // First destruct the table structs
        for (auto i = 0; i < CMD_DISPATCHER_DISPATCH_TABLE_SIZE; i++) {
            this->m_entryTable[i].~DispatchEntry();
        }
        for (auto i = 0; i < CMD_DISPATCHER_SEQUENCER_TABLE_SIZE; i++) {
            this->m_sequenceTracker[i].~SequenceTracker();
        }
        // Then deallocate the memory
        if (this->m_allocator != nullptr) {
            void* memory = static_cast<void*>(this->m_entryTable);
            this->m_allocator->deallocate(this->m_memId, memory);
        }
    }
}

void PassiveCmdDispatcher::setup(FwEnumStoreType memId, Fw::MemAllocator& allocator) {
    FW_ASSERT(!this->m_setupDone);
    this->m_allocator = &allocator;
    this->m_memId = memId;

    // Allocate a chunk of memory for both the dispatch table and sequence tracker
    FwSizeType expected_size = (CMD_DISPATCHER_DISPATCH_TABLE_SIZE * sizeof(DispatchEntry)) +
                               (CMD_DISPATCHER_SEQUENCER_TABLE_SIZE * sizeof(SequenceTracker));
    FwSizeType allocated_size = expected_size;
    bool recoverable = false;
    void* memory = allocator.allocate(memId, allocated_size, recoverable);
    FW_ASSERT((memory != nullptr) && (allocated_size == expected_size), allocated_size);

    // Initialize dispatch table entries
    this->m_entryTable = static_cast<DispatchEntry*>(memory);
    for (auto i = 0; i < CMD_DISPATCHER_DISPATCH_TABLE_SIZE; i++) {
        void* address = static_cast<void*>(this->m_entryTable + i);
        new (address) DispatchEntry();
    }
    for (U32 entry = 0; entry < CMD_DISPATCHER_DISPATCH_TABLE_SIZE; entry++) {
        this->m_entryTable[entry].opcode = OPCODE_UNUSED;
    }

    // Initialize sequence tracker entries
    void* dispatch_table_end = static_cast<void*>(this->m_entryTable + CMD_DISPATCHER_DISPATCH_TABLE_SIZE);
    this->m_sequenceTracker = static_cast<SequenceTracker*>(dispatch_table_end);
    for (auto i = 0; i < CMD_DISPATCHER_SEQUENCER_TABLE_SIZE; i++) {
        void* address = static_cast<void*>(this->m_sequenceTracker + i);
        new (address) SequenceTracker();
    }
    for (U32 entry = 0; entry < CMD_DISPATCHER_SEQUENCER_TABLE_SIZE; entry++) {
        this->m_sequenceTracker[entry].opcode = OPCODE_UNUSED;
    }

    this->m_setupDone = true;
}

// ----------------------------------------------------------------------
// Handler implementations for typed input ports
// ----------------------------------------------------------------------

void PassiveCmdDispatcher::compCmdReg_handler(FwIndexType portNum, FwOpcodeType opCode) {
    FW_ASSERT(this->m_setupDone);
    // This opcode is reserved for internal use
    FW_ASSERT(opCode != OPCODE_UNUSED);

    // Search for an empty slot
    bool slotFound = false;
    for (FwOpcodeType slot = 0; slot < CMD_DISPATCHER_DISPATCH_TABLE_SIZE; slot++) {
        if ((this->m_entryTable[slot].opcode == OPCODE_UNUSED) && (!slotFound)) {
            // Empty slot found
            this->m_entryTable[slot].opcode = opCode;
            this->m_entryTable[slot].port = portNum;
            slotFound = true;
        } else if ((this->m_entryTable[slot].opcode == opCode) && (this->m_entryTable[slot].port == portNum) &&
                   (!slotFound)) {
            // Found a slot where the opcode was already registered
            slotFound = true;
        } else if (this->m_entryTable[slot].opcode != OPCODE_UNUSED) {
            // Ensure that there are no duplicates
            FW_ASSERT(this->m_entryTable[slot].opcode != opCode, static_cast<FwAssertArgType>(opCode));
        }
    }
    FW_ASSERT(slotFound, static_cast<FwAssertArgType>(opCode));
}

void PassiveCmdDispatcher::compCmdStat_handler(FwIndexType portNum,
                                               FwOpcodeType opCode,
                                               U32 cmdSeq,
                                               const Fw::CmdResponse& response) {
    FW_ASSERT(this->m_setupDone);
    // This opcode is reserved for internal use
    FW_ASSERT(opCode != OPCODE_UNUSED);

    // Check the command response and log success/failure
    if (response.e == Fw::CmdResponse::OK) {
        this->log_COMMAND_OpCodeCompleted(opCode);
    } else {
        FW_ASSERT(response.e != Fw::CmdResponse::OK);
        this->log_COMMAND_OpCodeError(opCode, response);
    }

    // Search for the command source
    FwIndexType portToCall = -1;
    U32 context;
    for (U32 pending = 0; pending < CMD_DISPATCHER_SEQUENCER_TABLE_SIZE; pending++) {
        if ((this->m_sequenceTracker[pending].seq == cmdSeq) &&
            (this->m_sequenceTracker[pending].opcode != OPCODE_UNUSED)) {
            portToCall = this->m_sequenceTracker[pending].callerPort;
            context = this->m_sequenceTracker[pending].context;
            FW_ASSERT(opCode == this->m_sequenceTracker[pending].opcode);
            FW_ASSERT(portToCall < this->getNum_seqCmdStatus_OutputPorts());
            this->m_sequenceTracker[pending].opcode = OPCODE_UNUSED;
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
    FW_ASSERT(this->m_setupDone);

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
    // This opcode is reserved for internal use
    FW_ASSERT(opcode != OPCODE_UNUSED);

    // Search for the opcode in the dispatch table
    FwOpcodeType entry;
    bool entryFound = false;
    for (entry = 0; entry < CMD_DISPATCHER_DISPATCH_TABLE_SIZE; entry++) {
        if (this->m_entryTable[entry].opcode == opcode) {
            entryFound = true;
            break;
        }
    }
    if (entryFound && this->isConnected_compCmdSend_OutputPort(this->m_entryTable[entry].port)) {
        // Register the command in the command tracker only if the response port is connected
        if (this->isConnected_seqCmdStatus_OutputPort(portNum)) {
            bool pendingFound = false;
            for (U32 pending = 0; pending < CMD_DISPATCHER_SEQUENCER_TABLE_SIZE; pending++) {
                if (this->m_sequenceTracker[pending].opcode == OPCODE_UNUSED) {
                    pendingFound = true;
                    this->m_sequenceTracker[pending].opcode = opcode;
                    this->m_sequenceTracker[pending].seq = this->m_seq;
                    this->m_sequenceTracker[pending].context = context;
                    this->m_sequenceTracker[pending].callerPort = portNum;
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
        this->compCmdSend_out(this->m_entryTable[entry].port, opcode, this->m_seq, cmdPkt.getArgBuffer());
        this->log_COMMAND_OpCodeDispatched(opcode, this->m_entryTable[entry].port);
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
    FW_ASSERT(this->m_setupDone);

    this->log_ACTIVITY_HI_NoOpReceived();
    this->cmdResponse_out(opCode, cmdSeq, Fw::CmdResponse::OK);
}

void PassiveCmdDispatcher::CMD_CLEAR_TRACKING_cmdHandler(FwOpcodeType opCode, U32 cmdSeq) {
    FW_ASSERT(this->m_setupDone);

    // Clear the sequence tracking table
    for (FwOpcodeType entry = 0; entry < CMD_DISPATCHER_SEQUENCER_TABLE_SIZE; entry++) {
        this->m_sequenceTracker[entry].opcode = OPCODE_UNUSED;
    }
    this->cmdResponse_out(opCode, cmdSeq, Fw::CmdResponse::OK);
}

}  // namespace Baremetal
