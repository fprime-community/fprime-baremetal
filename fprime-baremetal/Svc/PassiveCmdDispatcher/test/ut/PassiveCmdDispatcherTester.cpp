// ======================================================================
// \title  PassiveCmdDispatcherTester.cpp
// \brief  cpp file for PassiveCmdDispatcher rules-based test harness
// ======================================================================

#include "PassiveCmdDispatcherTester.hpp"

#include <Fw/Com/ComPacket.hpp>

#include <cstring>
#include <limits>
#include <new>

namespace Baremetal {

// ----------------------------------------------------------------------
// Construction and destruction
// ----------------------------------------------------------------------

PassiveCmdDispatcherTester::PassiveCmdDispatcherTester()
    : PassiveCmdDispatcherGTestBase("Tester", MAX_HISTORY_SIZE),
      // The component constructor intentionally leaves its table and allocator pointers unset, so construct it in
      // zeroed storage to give setup() a known (nullptr) starting state.
      component(*new (std::memset(m_componentStorage, 0, sizeof(m_componentStorage)))
                    PassiveCmdDispatcher("PassiveCmdDispatcher")),
      m_nextSeq(0),
      m_cmdSendPort(-1),
      m_seqStatusPort(-1) {
    this->initComponents();
    this->connectPorts();
}

PassiveCmdDispatcherTester::~PassiveCmdDispatcherTester() {
    this->component.~PassiveCmdDispatcher();
}

// ----------------------------------------------------------------------
// Tests
// ----------------------------------------------------------------------

void PassiveCmdDispatcherTester::testNominal() {
    RegisterCommand registerFirst(0);
    RegisterCommand registerMid(CmdDispatcherComponentCommandPorts >> 1);
    ReRegisterCommand reregister;
    DispatchCommand dispatchCmd(false, 0);
    DispatchCommand dispatchBuff(true, 1);
    DispatchCommand dispatchNoStatus(false, UNCONNECTED_SEQ_PORT);
    CommandStatus statusOk(Fw::CmdResponse::OK);
    CommandStatus statusError(Fw::CmdResponse::EXECUTION_ERROR);
    UntrackedStatus untrackedStatus;
    NoOp noOp;
    ClearTracking clearTracking;

    registerFirst.apply(*this);
    registerMid.apply(*this);
    reregister.apply(*this);
    dispatchCmd.apply(*this);
    dispatchBuff.apply(*this);
    statusOk.apply(*this);
    statusError.apply(*this);
    // Commands from a source without a status port are dispatched but not tracked
    dispatchNoStatus.apply(*this);
    untrackedStatus.apply(*this);
    noOp.apply(*this);
    // Clearing tracking drops pending commands; their late status is not forwarded
    dispatchCmd.apply(*this);
    dispatchBuff.apply(*this);
    clearTracking.apply(*this);
    untrackedStatus.apply(*this);
}

void PassiveCmdDispatcherTester::testOffNominal() {
    RegisterCommand registerNoSend(UNCONNECTED_CMD_PORT);
    DispatchCommand dispatchCmd(false, 0);
    InvalidOpcode invalidOpcode(MAX_REGISTERED_OPCODE);
    InvalidOpcode reservedOpcode(std::numeric_limits<FwOpcodeType>::max());
    MalformedPacket malformed;

    invalidOpcode.apply(*this);
    reservedOpcode.apply(*this);
    malformed.apply(*this);
    // A registered opcode whose dispatch port is unconnected is rejected as invalid
    registerNoSend.apply(*this);
    dispatchCmd.apply(*this); // we haven't registered the port before this call
}

void PassiveCmdDispatcherTester::testTooManyCommands() {
    RegisterCommand registerFirst(0);
    RegisterCommand registerMid(CmdDispatcherComponentCommandPorts >> 1);
    DispatchCommand dispatchCmd(false, 0);
    DispatchCommand dispatchBuff(true, 1);
    CommandStatus statusOk(Fw::CmdResponse::OK);

    registerFirst.apply(*this);
    registerMid.apply(*this);
    // Fill the tracker with differing opcodes, dispatch ports, and caller ports
    for (U32 i = 0; i < CMD_DISPATCHER_SEQUENCER_TABLE_SIZE; i++) {
        if ((i % 2) == 0) {
            dispatchCmd.apply(*this);
        } else {
            dispatchBuff.apply(*this);
        }
    }
    ASSERT_EQ(static_cast<size_t>(CMD_DISPATCHER_SEQUENCER_TABLE_SIZE), this->m_pending.size());
    // Tracker is full so the command is rejected, until a status frees an entry
    dispatchCmd.apply(*this);
    statusOk.apply(*this); // drain one slot
    dispatchCmd.apply(*this);
}

void PassiveCmdDispatcherTester::testFailuresWhilePending() {
    RegisterCommand registerFirst(0);
    DispatchCommand dispatchCmd(false, 0);
    InvalidOpcode invalidOpcode(MAX_REGISTERED_OPCODE);
    MalformedPacket malformed;
    CommandStatus statusOk(Fw::CmdResponse::OK);

    registerFirst.apply(*this);
    dispatchCmd.apply(*this);
    // Rejected commands between a dispatch and its status must not disturb the tracked entries. 
    invalidOpcode.apply(*this);
    malformed.apply(*this);
    dispatchCmd.apply(*this);
    // Both tracked commands still report to the caller that sent them
    statusOk.apply(*this);
    statusOk.apply(*this);
}

// ----------------------------------------------------------------------
// Port invocations checked against the model
// ----------------------------------------------------------------------

void PassiveCmdDispatcherTester::invokeRegister(FwOpcodeType opcode, FwIndexType port) {
    this->clearHistory();
    this->invoke_to_compCmdReg(port, opcode);
    ASSERT_EVENTS_SIZE(0);
    this->m_registered[opcode] = port;
}

void PassiveCmdDispatcherTester::invokeDispatch(bool useBuffer, FwIndexType port, FwOpcodeType opcode, U32 context) {
    // The context doubles as the command argument payload
    Fw::CmdArgBuffer args;
    ASSERT_EQ(Fw::FW_SERIALIZE_OK, args.serializeFrom(context));

    this->clearHistory();
    if (useBuffer) {
        Fw::ComBuffer buff;
        ASSERT_EQ(Fw::FW_SERIALIZE_OK,
                  buff.serializeFrom(static_cast<FwPacketDescriptorType>(Fw::ComPacketType::FW_PACKET_COMMAND)));
        ASSERT_EQ(Fw::FW_SERIALIZE_OK, buff.serializeFrom(opcode));
        ASSERT_EQ(Fw::FW_SERIALIZE_OK, buff.serializeFrom(context));
        this->invoke_to_seqCmdBuff(port, buff, context);
    } else {
        this->invoke_to_seqCmdIn(port, opcode, context, args);
    }

    const auto entry = this->m_registered.find(opcode);
    ASSERT_EVENTS_SIZE(1);
    if ((entry == this->m_registered.end()) || (entry->second == UNCONNECTED_CMD_PORT)) {
        // Check we properly respond with `Fw::CmdResponse::INVALID_OPCODE` if we provided an unregistered opcode or deliberately unadded dispatch port
        ASSERT_EVENTS_InvalidCommand(0, opcode);
        ASSERT_from_compCmdSend_SIZE(0);
        this->checkSeqStatus(port, opcode, context, Fw::CmdResponse::INVALID_OPCODE);
        this->m_nextSeq++;
    } else if ((port != UNCONNECTED_SEQ_PORT) &&
               (this->m_pending.size() == static_cast<size_t>(CMD_DISPATCHER_SEQUENCER_TABLE_SIZE))) {
        // Check if the sequence tracker table is full
        ASSERT_EVENTS_TooManyCommands(0, opcode);
        ASSERT_from_compCmdSend_SIZE(0);
        this->checkSeqStatus(port, opcode, context, Fw::CmdResponse::EXECUTION_ERROR);
    } else {
        ASSERT_EVENTS_OpCodeDispatched(0, opcode, entry->second);
        ASSERT_from_compCmdSend_SIZE(1);
        ASSERT_from_compCmdSend(0, opcode, this->m_nextSeq, args);
        ASSERT_EQ(entry->second, this->m_cmdSendPort);
        ASSERT_from_seqCmdStatus_SIZE(0);
        // Only commands whose source can receive a status are tracked
        if (port != UNCONNECTED_SEQ_PORT) {
            this->m_pending[this->m_nextSeq] = {opcode, port, context};
        }
        this->m_nextSeq++;
    }
}

void PassiveCmdDispatcherTester::invokeMalformed(FwIndexType port, U32 context) {
    // A telemetry packet descriptor does not deserialize as a command packet
    Fw::ComBuffer buff;
    // Fw::CmdPacket::deserializeFrom checks for Fw::ComPacketType::FW_PACKET_COMMAND, so serializing Fw::ComPacketType::FW_PACKET_TELEM should fail in the sequence command buffer handler.
    ASSERT_EQ(Fw::FW_SERIALIZE_OK,
              buff.serializeFrom(static_cast<FwPacketDescriptorType>(Fw::ComPacketType::FW_PACKET_TELEM)));

    this->clearHistory();
    this->invoke_to_seqCmdBuff(port, buff, context);
    ASSERT_EVENTS_SIZE(1);
    ASSERT_EVENTS_MalformedCommand(0, Fw::DeserialStatus::TYPE_MISMATCH);
    ASSERT_from_compCmdSend_SIZE(0);
    // The opcode is never deserialized, so the packet's default opcode (0) is reported
    this->checkSeqStatus(port, 0, context, Fw::CmdResponse::VALIDATION_ERROR);
}

void PassiveCmdDispatcherTester::invokeStatus(FwOpcodeType opcode, U32 cmdSeq, Fw::CmdResponse response) {
    this->clearHistory();
    this->invoke_to_compCmdStat(0, opcode, cmdSeq, response);
    ASSERT_EVENTS_SIZE(1);
    if (response == Fw::CmdResponse::OK) {
        ASSERT_EVENTS_OpCodeCompleted(0, opcode);
    } else {
        ASSERT_EVENTS_OpCodeError(0, opcode, response);
    }

    const auto pending = this->m_pending.find(cmdSeq);
    if (pending == this->m_pending.end()) {
        ASSERT_from_seqCmdStatus_SIZE(0);
    } else {
        this->checkSeqStatus(pending->second.port, opcode, pending->second.context, response);
        // Clear the pending status
        this->m_pending.erase(pending);
    }
}

void PassiveCmdDispatcherTester::invokeNoOp(U32 cmdSeq) {
    this->clearHistory();
    this->sendCmd_CMD_NO_OP(TEST_INSTANCE_ID, cmdSeq);
    ASSERT_EVENTS_SIZE(1);
    ASSERT_EVENTS_NoOpReceived_SIZE(1);
    ASSERT_CMD_RESPONSE_SIZE(1);
    ASSERT_CMD_RESPONSE(0, PassiveCmdDispatcherComponentBase::OPCODE_CMD_NO_OP, cmdSeq, Fw::CmdResponse::OK);
}

void PassiveCmdDispatcherTester::invokeClearTracking(U32 cmdSeq) {
    this->clearHistory();
    this->sendCmd_CMD_CLEAR_TRACKING(TEST_INSTANCE_ID, cmdSeq);
    ASSERT_EVENTS_SIZE(0);
    ASSERT_from_seqCmdStatus_SIZE(0);
    ASSERT_CMD_RESPONSE_SIZE(1);
    ASSERT_CMD_RESPONSE(0, PassiveCmdDispatcherComponentBase::OPCODE_CMD_CLEAR_TRACKING, cmdSeq,
                        Fw::CmdResponse::OK);
    this->m_pending.clear();
}

void PassiveCmdDispatcherTester::checkSeqStatus(FwIndexType port,
                                                FwOpcodeType opcode,
                                                U32 context,
                                                Fw::CmdResponse response) {
    if (port == UNCONNECTED_SEQ_PORT) {
        ASSERT_from_seqCmdStatus_SIZE(0);
    } else {
        ASSERT_from_seqCmdStatus_SIZE(1);
        ASSERT_from_seqCmdStatus(0, opcode, context, response);
        ASSERT_EQ(port, this->m_seqStatusPort);
    }
}

FwOpcodeType PassiveCmdDispatcherTester::pickUnregisteredOpcode() const {
    FwOpcodeType opcode = 0;
    do {
        opcode = STest::Pick::startLength(0, MAX_REGISTERED_OPCODE);
    } while (this->m_registered.count(opcode) != 0);
    return opcode;
}

// ----------------------------------------------------------------------
// Handlers for typed from ports
// ----------------------------------------------------------------------

void PassiveCmdDispatcherTester::from_compCmdSend_handler(FwIndexType portNum,
                                                          FwOpcodeType opCode,
                                                          U32 cmdSeq,
                                                          Fw::CmdArgBuffer& args) {
    this->pushFromPortEntry_compCmdSend(opCode, cmdSeq, args);
    this->m_cmdSendPort = portNum;
}

void PassiveCmdDispatcherTester::from_seqCmdStatus_handler(FwIndexType portNum,
                                                           FwOpcodeType opCode,
                                                           U32 cmdSeq,
                                                           const Fw::CmdResponse& response) {
    this->pushFromPortEntry_seqCmdStatus(opCode, cmdSeq, response);
    this->m_seqStatusPort = portNum;
}

// ----------------------------------------------------------------------
// Helper methods
// ----------------------------------------------------------------------

void PassiveCmdDispatcherTester::connectPorts() {
    // Special ports
    this->connect_to_cmdIn(0, this->component.get_cmdIn_InputPort(0));
    this->component.set_cmdRegOut_OutputPort(0, this->get_from_cmdRegOut(0));
    this->component.set_cmdResponseOut_OutputPort(0, this->get_from_cmdResponseOut(0));
    this->component.set_logOut_OutputPort(0, this->get_from_logOut(0));
#if FW_ENABLE_TEXT_LOGGING == 1
    this->component.set_logTextOut_OutputPort(0, this->get_from_logTextOut(0));
#endif
    this->component.set_timeCaller_OutputPort(0, this->get_from_timeCaller(0));

    // Command registration, dispatch, and status ports
    for (FwIndexType i = 0; i < CmdDispatcherComponentCommandPorts; i++) {
        this->connect_to_compCmdReg(i, this->component.get_compCmdReg_InputPort(i));
        if (i != UNCONNECTED_CMD_PORT) {
            this->component.set_compCmdSend_OutputPort(i, this->get_from_compCmdSend(i));
        }
    }
    this->connect_to_compCmdStat(0, this->component.get_compCmdStat_InputPort(0));

    // Command source ports
    for (FwIndexType i = 0; i < CmdDispatcherSequencePorts; i++) {
        this->connect_to_seqCmdBuff(i, this->component.get_seqCmdBuff_InputPort(i));
        this->connect_to_seqCmdIn(i, this->component.get_seqCmdIn_InputPort(i));
        if (i != UNCONNECTED_SEQ_PORT) {
            this->component.set_seqCmdStatus_OutputPort(i, this->get_from_seqCmdStatus(i));
        }
    }
}

void PassiveCmdDispatcherTester::initComponents() {
    this->init();
    this->component.init(TEST_INSTANCE_ID);
    this->component.setup(0, this->m_allocator);
}

}  // namespace Baremetal
