// ======================================================================
// \title  TlmLinearChan/test/ut/TlmLinearChanTester.hpp
// \author ethanchee
// \brief  hpp file for TlmLinearChan test harness implementation class
// ======================================================================

#ifndef TLMLINEARCHAN_TESTER_HPP
#define TLMLINEARCHAN_TESTER_HPP

#include "TlmLinearChanGTestBase.hpp"
#include "fprime-baremetal/Svc/TlmLinearChan/TlmLinearChan.hpp"

#include <Fw/Types/MallocAllocator.hpp>

namespace Baremetal {

class TlmLinearChanTester : public TlmLinearChanGTestBase {
    // ----------------------------------------------------------------------
    // Construction and destruction
    // ----------------------------------------------------------------------

  public:
    //! Construct object Tester
    //!
    TlmLinearChanTester();

    //! Destroy object Tester
    //!
    ~TlmLinearChanTester();

  public:
    // ----------------------------------------------------------------------
    // Tests
    // ----------------------------------------------------------------------

    void runNominalChannel();
    void runMultiChannel();
    void runOffNominal();

  private:
    // ----------------------------------------------------------------------
    // Handlers for typed from ports
    // ----------------------------------------------------------------------

    //! Handler for from_PktSend
    //!
    void from_PktSend_handler(FwIndexType portNum, /*!< The port number*/
                              Fw::ComBuffer& data, /*!<
                          Buffer containing packet data
                          */
                              U32 context          /*!<
                                   Call context value; meaning chosen by user
                                   */
    );

    //! Handler for from_pingOut
    //!
    void from_pingOut_handler(FwIndexType portNum, /*!< The port number*/
                              U32 key              /*!<
                                       Value to return to pinger
                                       */
    );

  private:
    // ----------------------------------------------------------------------
    // Helper methods
    // ----------------------------------------------------------------------

    //! Connect ports
    //!
    void connectPorts();

    //! Initialize components
    //!
    void initComponents();

    void sendBuff(FwChanIdType id, U32 val);
    bool doRun(bool check);
    void checkBuff(FwChanIdType chanNum, FwChanIdType totalChan, FwChanIdType id, U32 val);

    void clearBuffs();

    // dump functions
    static void dumpTlmEntry(TlmLinearChan::TlmEntry* entry);

  private:
    // ----------------------------------------------------------------------
    // Variables
    // ----------------------------------------------------------------------

    // Note: This MUST be declared before 'component' or it will be destroyed
    // before the component can deallocate its memory.
    Fw::MallocAllocator m_mallocator;

    //! The component under test
    //!
    TlmLinearChan component;

    // Keep a history
    FwIndexType m_numBuffs;
    Fw::ComBuffer m_rcvdBuffer[TLMCHAN_HASH_BUCKETS];
    bool m_bufferRecv;
};

}  // end namespace Baremetal

#endif
