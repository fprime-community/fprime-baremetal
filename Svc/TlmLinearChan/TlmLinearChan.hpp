// ======================================================================
// \title  TlmLinearChan.hpp
// \author ethanchee
// \brief  Component that stores telemetry channel values
// ======================================================================

#ifndef TELEMCHANIMPL_HPP_
#define TELEMCHANIMPL_HPP_

#include <Fw/Tlm/TlmPacket.hpp>
#include <Svc/TlmLinearChan/TlmLinearChanComponentAc.hpp>
#include <TlmChanImplCfg.hpp>

namespace Baremetal {

class TlmLinearChan : public TlmLinearChanComponentBase {
  public:
    TlmLinearChan(const char* compName);
    virtual ~TlmLinearChan();
    void init(NATIVE_INT_TYPE queueDepth, /*!< The queue depth*/
              NATIVE_INT_TYPE instance    /*!< The instance number*/
    );

  PRIVATE:
    // Port functions
    void TlmRecv_handler(NATIVE_INT_TYPE portNum, FwChanIdType id, Fw::Time& timeTag, Fw::TlmBuffer& val);
    void TlmGet_handler(NATIVE_INT_TYPE portNum, FwChanIdType id, Fw::Time& timeTag, Fw::TlmBuffer& val);
    void Run_handler(NATIVE_INT_TYPE portNum, NATIVE_UINT_TYPE context);
    //! Handler implementation for pingIn
    //!
    void pingIn_handler(const NATIVE_INT_TYPE portNum, /*!< The port number*/
                        U32 key                        /*!< Value to return to pinger*/
    );

    typedef struct tlmEntry {
        FwChanIdType id;  //!< telemetry id stored in slot
        bool updated;     //!< set whenever a value has been written. Used to skip if writing out values for downlinking
        Fw::Time lastUpdate;        //!< last updated time
        Fw::TlmBuffer buffer;       //!< buffer to store serialized telemetry
        bool used;                  //!< if entry has been used
    } TlmEntry;

    TlmEntry m_tlmEntries[TLMCHAN_HASH_BUCKETS];
};

}  // namespace TlmLinearChan

#endif /* TELEMCHANIMPL_HPP_ */
