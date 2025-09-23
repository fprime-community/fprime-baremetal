// ======================================================================
// \title  PassiveCmdDispatcher.hpp
// \author root
// \brief  hpp file for PassiveCmdDispatcher component implementation class
// ======================================================================

#ifndef Baremetal_PassiveCmdDispatcher_HPP
#define Baremetal_PassiveCmdDispatcher_HPP

#include <Fw/Types/MemAllocator.hpp>
#include <Svc/PassiveCmdDispatcher/PassiveCmdDispatcherComponentAc.hpp>
#include <config/CommandDispatcherImplCfg.hpp>

namespace Baremetal {

class PassiveCmdDispatcher final : public PassiveCmdDispatcherComponentBase {
  public:
    // ----------------------------------------------------------------------
    // Component construction and destruction
    // ----------------------------------------------------------------------

    //! Construct PassiveCmdDispatcher object
    PassiveCmdDispatcher(const char* const compName  //!< The component name
    );

    //! Destroy PassiveCmdDispatcher object
    ~PassiveCmdDispatcher();

    //! Allocate memory and set up buffers
    void setup(FwEnumStoreType memId,       //!< Memory segment identifier
               Fw::MemAllocator& allocator  //!< Memory allocator
    );

  private:
    // ----------------------------------------------------------------------
    // Handler implementations for typed input ports
    // ----------------------------------------------------------------------

    //! Handler implementation for compCmdReg
    //!
    //! Command registration input ports. Size must match the dispatch output ports.
    void compCmdReg_handler(FwIndexType portNum,  //!< The port number
                            FwOpcodeType opCode   //!< Command Op Code
                            ) override;

    //! Handler implementation for compCmdStat
    //!
    //! Input command status ports
    void compCmdStat_handler(FwIndexType portNum,             //!< The port number
                             FwOpcodeType opCode,             //!< Command Op Code
                             U32 cmdSeq,                      //!< Command Sequence
                             const Fw::CmdResponse& response  //!< The command response argument
                             ) override;

    //! Handler implementation for seqCmdBuff
    //!
    //! Command buffer input port for sequencers or other sources of command buffers
    void seqCmdBuff_handler(FwIndexType portNum,  //!< The port number
                            Fw::ComBuffer& data,  //!< Buffer containing packet data
                            U32 context           //!< Call context value; meaning chosen by user
                            ) override;

  private:
    // ----------------------------------------------------------------------
    // Handler implementations for commands
    // ----------------------------------------------------------------------

    //! Handler implementation for command CMD_NO_OP
    //!
    //! No-op command
    void CMD_NO_OP_cmdHandler(FwOpcodeType opCode,  //!< The opcode
                              U32 cmdSeq            //!< The command sequence number
                              ) override;

    //! Handler implementation for command CMD_CLEAR_TRACKING
    //!
    //! Clear command tracking info to recover from components that are not returning status
    void CMD_CLEAR_TRACKING_cmdHandler(FwOpcodeType opCode,  //!< The opcode
                                       U32 cmdSeq            //!< The command sequence number
                                       ) override;

    struct DispatchEntry {
        FwOpcodeType opcode;  //!< opcode of entry
        FwIndexType port;     //!< which port the entry invokes
    };

    struct SequenceTracker {
        U32 seq;                 //!< command sequence number
        FwOpcodeType opcode;     //!< opcode being tracked
        U32 context;             //!< context passed by user
        FwIndexType callerPort;  //!< port command source port
    };

    //!< Current command sequence number
    // TODO: this could be sized down but would require fprime core changes to typedef the sequence
    // value, currently it is hard-coded to a U32 in many type/port definitions
    U32 m_seq;
    //! Dispatch entry table: maps incoming opcodes to the port connected to the component that
    //! implements the command
    //! size: CMD_DISPATCHER_DISPATCH_TABLE_SIZE
    DispatchEntry* m_entryTable;
    //! Sequence tracker table: tracks commands that are being executed but are not yet complete
    //! size: CMD_DISPATCHER_SEQUENCER_TABLE_SIZE
    SequenceTracker* m_sequenceTracker;

    //! Memory allocator and region ID
    Fw::MemAllocator* m_allocator;
    FwEnumStoreType m_memId;
    //! Flag to indicate that the setup function has been called and the memory for the dispatch
    //! entry table and sequence tracker been allocated
    bool m_setupDone;
};

}  // namespace Baremetal

#endif  // Baremetal_PassiveCmdDispatcher_HPP
