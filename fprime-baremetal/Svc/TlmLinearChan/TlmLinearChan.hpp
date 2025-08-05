// ======================================================================
// \title  TlmLinearChan.hpp
// \author ethanchee
// \brief  Component that stores telemetry channel values
// ======================================================================

#ifndef TELEMCHANIMPL_HPP_
#define TELEMCHANIMPL_HPP_

#include <Fw/Tlm/TlmPacket.hpp>
#include <Fw/Types/MemAllocator.hpp>
#include <fprime-baremetal/Svc/TlmLinearChan/TlmLinearChanComponentAc.hpp>
#include <config/TlmChanImplCfg.hpp>

namespace Baremetal {

class TlmLinearChan : public TlmLinearChanComponentBase {
  friend class TlmLinearChanTester;

  public:
    TlmLinearChan(const char* compName);
    virtual ~TlmLinearChan();
    void init(FwSizeType queueDepth, /*!< The queue depth*/
              FwEnumStoreType instance    /*!< The instance number*/
    );
    void setup(FwEnumStoreType memId,       //!< Memory segment identifier
               Fw::MemAllocator& allocator  //!< Memory allocator
    );

  private:
    // Port functions
    void TlmRecv_handler(FwIndexType portNum, FwChanIdType id, Fw::Time& timeTag, Fw::TlmBuffer& val);
    Fw::TlmValid TlmGet_handler(FwIndexType portNum, FwChanIdType id, Fw::Time& timeTag, Fw::TlmBuffer& val);
    void Run_handler(FwIndexType portNum, U32 context);
    //! Handler implementation for pingIn
    //!
    void pingIn_handler(const FwIndexType portNum, /*!< The port number*/
                        U32 key                    /*!< Value to return to pinger*/
    );

    typedef struct tlmEntry {
        FwChanIdType id;  //!< telemetry id stored in slot
        bool updated;     //!< set whenever a value has been written. Used to skip if writing out values for downlinking
        Fw::Time lastUpdate;        //!< last updated time
        Fw::TlmBuffer buffer;       //!< buffer to store serialized telemetry
        bool used;                  //!< if entry has been used
    } TlmEntry;

    Fw::MemAllocator* m_allocator;
    FwEnumStoreType m_memId;
    TlmEntry* m_tlmEntries;
    // Flag to indicate that setup(...) has been called and memory for m_tlmEntries has been allocated
    bool m_setupDone;
};

}  // namespace TlmLinearChan

#endif /* TELEMCHANIMPL_HPP_ */
