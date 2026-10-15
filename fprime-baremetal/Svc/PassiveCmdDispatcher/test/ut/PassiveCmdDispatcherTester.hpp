// ======================================================================
// \title  PassiveCmdDispatcherTester.hpp
// \brief  hpp file for PassiveCmdDispatcher rules-based test harness
// ======================================================================

#ifndef Baremetal_PassiveCmdDispatcherTester_HPP
#define Baremetal_PassiveCmdDispatcherTester_HPP

#include <Fw/Types/MallocAllocator.hpp>
#include <fprime-baremetal/Svc/PassiveCmdDispatcher/PassiveCmdDispatcher.hpp>

#include <map>

#include "PassiveCmdDispatcherGTestBase.hpp"
#include "RulesHeaders.hpp"

namespace Baremetal {

class PassiveCmdDispatcherTester : public PassiveCmdDispatcherGTestBase {
  public:
    enum {
        MAX_HISTORY_SIZE = 10,
        TEST_INSTANCE_ID = 0,
        //! Last port of each array is left unconnected to exercise the isConnected checks
        UNCONNECTED_CMD_PORT = CmdDispatcherComponentCommandPorts - 1,
        UNCONNECTED_SEQ_PORT = CmdDispatcherSequencePorts - 1,
        //! Rules only register opcodes below this value, so opcodes at or above it are always invalid
        MAX_REGISTERED_OPCODE = 0x1000,
    };

    //! A command the model expects the dispatcher to be tracking
    struct Pending {
        FwOpcodeType opcode;
        FwIndexType port;
        U32 context;
    };

#include "MyRules.hpp"

    PassiveCmdDispatcherTester();
    ~PassiveCmdDispatcherTester();

    // ----------------------------------------------------------------------
    // Tests
    // ----------------------------------------------------------------------

    void testNominal();
    void testOffNominal();
    void testTooManyCommands();
    void testFailuresWhilePending();

  private:
    // ----------------------------------------------------------------------
    // Port invocations checked against the model
    // ----------------------------------------------------------------------

    void invokeRegister(FwOpcodeType opcode, FwIndexType port);
    void invokeDispatch(bool useBuffer, FwIndexType port, FwOpcodeType opcode, U32 context);
    void invokeMalformed(FwIndexType port, U32 context);
    void invokeStatus(FwOpcodeType opcode, U32 cmdSeq, Fw::CmdResponse response);
    void invokeNoOp(U32 cmdSeq);
    void invokeClearTracking(U32 cmdSeq);

    //! Check the seqCmdStatus output expected for a command source
    void checkSeqStatus(FwIndexType port, FwOpcodeType opcode, U32 context, Fw::CmdResponse response);

    //! Pick a random opcode below MAX_REGISTERED_OPCODE that is not yet registered
    FwOpcodeType pickUnregisteredOpcode() const;

    // ----------------------------------------------------------------------
    // Handlers for typed from ports
    // ----------------------------------------------------------------------

    void from_compCmdSend_handler(FwIndexType portNum, FwOpcodeType opCode, U32 cmdSeq, Fw::CmdArgBuffer& args);

    void from_seqCmdStatus_handler(FwIndexType portNum,
                                   FwOpcodeType opCode,
                                   U32 cmdSeq,
                                   const Fw::CmdResponse& response);

    // ----------------------------------------------------------------------
    // Helper methods
    // ----------------------------------------------------------------------

    void connectPorts();
    void initComponents();

    // ----------------------------------------------------------------------
    // Variables
    // ----------------------------------------------------------------------

    // Note: This must be declared before 'component' member or it will be destroyed before the component can deallocate its memory.
    Fw::MallocAllocator m_allocator;

    //! Zeroed storage 'component' is constructed in (see constructor)
    alignas(PassiveCmdDispatcher) U8 m_componentStorage[sizeof(PassiveCmdDispatcher)];

    //! The component under test
    PassiveCmdDispatcher& component;

    // Model of the dispatcher state
    std::map<FwOpcodeType, FwIndexType> m_registered;  //!< registered opcode -> compCmdSend port
    std::map<U32, Pending> m_pending;                  //!< tracked sequence number -> command
    U32 m_nextSeq;                                     //!< next sequence number the dispatcher assigns

    // Port numbers of the most recent output port calls
    FwIndexType m_cmdSendPort;
    FwIndexType m_seqStatusPort;
};

}  // namespace Baremetal

#endif
