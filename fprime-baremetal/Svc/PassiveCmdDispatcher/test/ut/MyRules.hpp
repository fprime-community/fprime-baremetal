// ======================================================================
// \title  MyRules.hpp
// \brief  Rules for PassiveCmdDispatcher; included inside PassiveCmdDispatcherTester
// ======================================================================

// ------------------------------------------------------------------------------------------------------
// Rule:  RegisterCommand
//
// Register a new opcode on a fixed compCmdReg port
// Excpects successful registration of (opcode, port)
// ------------------------------------------------------------------------------------------------------
struct RegisterCommand : public STest::Rule<PassiveCmdDispatcherTester> {
    RegisterCommand(FwIndexType port);
    bool precondition(const PassiveCmdDispatcherTester& state);
    void action(PassiveCmdDispatcherTester& state);

    FwIndexType port;
};

// ------------------------------------------------------------------------------------------------------
// Rule:  ReRegisterCommand
//
// Register an already registered opcode again on the same port
// Excpects successful re-registration of the same (opcode, port) already in the command entry table
// ------------------------------------------------------------------------------------------------------
struct ReRegisterCommand : public STest::Rule<PassiveCmdDispatcherTester> {
    ReRegisterCommand();
    bool precondition(const PassiveCmdDispatcherTester& state);
    void action(PassiveCmdDispatcherTester& state);
};

// ------------------------------------------------------------------------------------------------------
// Rule:  NonUniqueOpcodeFailure
// 
// Excpects failure to re-registrater the same opcode already present in the command entry table
// ------------------------------------------------------------------------------------------------------


// ------------------------------------------------------------------------------------------------------
// Rule:  DispatchCommand
//
// Send a registered opcode on a fixed sequence port via seqCmdIn or seqCmdBuff
// ------------------------------------------------------------------------------------------------------
struct DispatchCommand : public STest::Rule<PassiveCmdDispatcherTester> {
    DispatchCommand(bool useBuffer, FwIndexType port);
    bool precondition(const PassiveCmdDispatcherTester& state);
    void action(PassiveCmdDispatcherTester& state);

    bool useBuffer;
    FwIndexType port;
};

// ------------------------------------------------------------------------------------------------------
// Rule:  InvalidOpcode
//
// Send an opcode that was never registered
// ------------------------------------------------------------------------------------------------------
struct InvalidOpcode : public STest::Rule<PassiveCmdDispatcherTester> {
    InvalidOpcode(FwOpcodeType opcode);
    bool precondition(const PassiveCmdDispatcherTester& state);
    void action(PassiveCmdDispatcherTester& state);

    FwOpcodeType opcode;
};

// ------------------------------------------------------------------------------------------------------
// Rule:  MalformedPacket
//
// Send a command buffer that does not deserialize as a command packet
// ------------------------------------------------------------------------------------------------------
struct MalformedPacket : public STest::Rule<PassiveCmdDispatcherTester> {
    MalformedPacket();
    bool precondition(const PassiveCmdDispatcherTester& state);
    void action(PassiveCmdDispatcherTester& state);
};

// ------------------------------------------------------------------------------------------------------
// Rule:  CommandStatus
//
// Return a status for a command that is being tracked
// ------------------------------------------------------------------------------------------------------
struct CommandStatus : public STest::Rule<PassiveCmdDispatcherTester> {
    CommandStatus(Fw::CmdResponse response);
    bool precondition(const PassiveCmdDispatcherTester& state);
    void action(PassiveCmdDispatcherTester& state);

    Fw::CmdResponse response;
};

// ------------------------------------------------------------------------------------------------------
// Rule:  UntrackedStatus
//
// Return a status for a sequence number that is not being tracked
// ------------------------------------------------------------------------------------------------------
struct UntrackedStatus : public STest::Rule<PassiveCmdDispatcherTester> {
    UntrackedStatus();
    bool precondition(const PassiveCmdDispatcherTester& state);
    void action(PassiveCmdDispatcherTester& state);
};

// ------------------------------------------------------------------------------------------------------
// Rule:  NoOp
//
// Send the CMD_NO_OP command
// ------------------------------------------------------------------------------------------------------
struct NoOp : public STest::Rule<PassiveCmdDispatcherTester> {
    NoOp();
    bool precondition(const PassiveCmdDispatcherTester& state);
    void action(PassiveCmdDispatcherTester& state);
};

// ------------------------------------------------------------------------------------------------------
// Rule:  ClearTracking
//
// Send the CMD_CLEAR_TRACKING command
// ------------------------------------------------------------------------------------------------------
struct ClearTracking : public STest::Rule<PassiveCmdDispatcherTester> {
    ClearTracking();
    bool precondition(const PassiveCmdDispatcherTester& state);
    void action(PassiveCmdDispatcherTester& state);
};
