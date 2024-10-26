// ======================================================================
// \title  TlmLinearChan/test/ut/Tester.hpp
// \author ethanchee
// \brief  hpp file for TlmLinearChan test harness implementation class
// ======================================================================

#ifndef TESTER_HPP
#define TESTER_HPP

#include "GTestBase.hpp"
#include "Svc/TlmLinearChan/TlmLinearChan.hpp"

namespace Baremetal {

class Tester : public TlmLinearChanGTestBase {
    // ----------------------------------------------------------------------
    // Construction and destruction
    // ----------------------------------------------------------------------

  public:
    //! Construct object Tester
    //!
    Tester();

    //! Destroy object Tester
    //!
    ~Tester();

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
    void from_PktSend_handler(const NATIVE_INT_TYPE portNum, /*!< The port number*/
                              Fw::ComBuffer& data,           /*!<
                                    Buffer containing packet data
                                    */
                              U32 context                    /*!<
                                             Call context value; meaning chosen by user
                                             */
    );

    //! Handler for from_pingOut
    //!
    void from_pingOut_handler(const NATIVE_INT_TYPE portNum, /*!< The port number*/
                              U32 key                        /*!<
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
    void checkBuff(NATIVE_UINT_TYPE chanNum, NATIVE_UINT_TYPE totalChan, FwChanIdType id, U32 val);

    void clearBuffs();

    // dump functions
    static void dumpTlmEntry(TlmLinearChan::TlmEntry* entry);

  private:
    // ----------------------------------------------------------------------
    // Variables
    // ----------------------------------------------------------------------

    //! The component under test
    //!
    TlmLinearChan component;
    // Keep a history
    NATIVE_UINT_TYPE m_numBuffs;
    Fw::ComBuffer m_rcvdBuffer[TLMCHAN_HASH_BUCKETS];
    bool m_bufferRecv;
};

}  // end namespace Svc

#endif
