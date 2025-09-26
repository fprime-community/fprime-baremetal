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
  private:
    // ----------------------------------------------------------------------
    // Type definitions
    // ----------------------------------------------------------------------

    struct DispatchEntry {
        // unused entries have the opcode set to OPCODE_UNUSED (see .cpp for definition)
        FwOpcodeType opcode;  //!< opcode of entry
        FwIndexType port;     //!< which port the entry invokes
    };

    struct SequenceTracker {
        // unused entries have the opcode set to OPCODE_UNUSED (see .cpp for definition)
        FwOpcodeType opcode;     //!< opcode being tracked
        FwIndexType callerPort;  //!< port command source port
        U32 seq;                 //!< command sequence number
        U32 context;             //!< context passed by user
    };

    // Wraps the dispatch entry and sequence tracker tables for cleaner memory management
    struct CmdTables {
        CmdTables();  // initializes m_entryTable[i].opcode and m_sequenceTracker[i].opcode

        //! Dispatch entry table: maps incoming opcodes to the port connected to the component that
        //! implements the command
        DispatchEntry m_entryTable[CMD_DISPATCHER_DISPATCH_TABLE_SIZE];
        //! Sequence tracker table: tracks commands that are being executed but not yet complete
        SequenceTracker m_sequenceTracker[CMD_DISPATCHER_SEQUENCER_TABLE_SIZE];
    };

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

    //!< Current command sequence number
    // TODO: this could be sized down but would require fprime core changes to typedef the sequence
    // value, currently it is hard-coded to a U32 in many type/port definitions
    U32 m_seq;
    //! Contains dispatch entry table and sequence tracker, see type definitions for more details
    CmdTables* m_cmdTables;

    //! Memory allocator and region ID
    Fw::MemAllocator* m_allocator;
    FwEnumStoreType m_memId;
};

}  // namespace Baremetal

#endif  // Baremetal_PassiveCmdDispatcher_HPP
