// ======================================================================
// \title  TlmLinearChan.cpp
// \author ethanchee
// \brief  Implementation file for channelized telemetry storage component
// ======================================================================

#include <FpConfig.hpp>
#include <Fw/Com/ComBuffer.hpp>
#include <Fw/Types/Assert.hpp>
#include <fprime-baremetal/Svc/TlmLinearChan/TlmLinearChan.hpp>

namespace Baremetal {

TlmLinearChan::TlmLinearChan(const char* name) : TlmLinearChanComponentBase(name) {
    // clear buckets
    for (NATIVE_UINT_TYPE entry = 0; entry < TLMCHAN_HASH_BUCKETS; entry++) {
        this->m_tlmEntries[entry].updated = false;
        this->m_tlmEntries[entry].id = 0;
        this->m_tlmEntries[entry].used = false;
    }
}

TlmLinearChan::~TlmLinearChan() {}

void TlmLinearChan::init(NATIVE_INT_TYPE queueDepth, /*!< The queue depth*/
                   NATIVE_INT_TYPE instance    /*!< The instance number*/
) {
    TlmLinearChanComponentBase::init(queueDepth, instance);
}

void TlmLinearChan::pingIn_handler(const NATIVE_INT_TYPE portNum, U32 key) {
    // return key
    this->pingOut_out(0, key);
}

void TlmLinearChan::TlmGet_handler(NATIVE_INT_TYPE portNum, FwChanIdType id, Fw::Time& timeTag, Fw::TlmBuffer& val) {
    // Compute index for entry

    NATIVE_UINT_TYPE entry;
    for (entry = 0; entry < TLMCHAN_HASH_BUCKETS; entry++) {
        if (this->m_tlmEntries[entry].id == id) {  // If bucket exists, check id
            break;
        }
    }

    if(entry < TLMCHAN_HASH_BUCKETS)
    {
        val = this->m_tlmEntries[entry].buffer;
        timeTag = this->m_tlmEntries[entry].lastUpdate;
    } else 
    {  // requested entry may not be written yet; empty buffer
        val.resetSer();
    }
}

void TlmLinearChan::TlmRecv_handler(NATIVE_INT_TYPE portNum, FwChanIdType id, Fw::Time& timeTag, Fw::TlmBuffer& val) {
    // Compute index for entry

    NATIVE_UINT_TYPE entry;
    for (entry = 0; entry < TLMCHAN_HASH_BUCKETS; entry++) {
        if (this->m_tlmEntries[entry].id == id || this->m_tlmEntries[entry].used == false) {  
            break;
        }
    }

    // copy into entry
    FW_ASSERT(entry < TLMCHAN_HASH_BUCKETS, entry);
    this->m_tlmEntries[entry].used = true;
    this->m_tlmEntries[entry].id = id;
    this->m_tlmEntries[entry].updated = true;
    this->m_tlmEntries[entry].lastUpdate = timeTag;
    this->m_tlmEntries[entry].buffer = val;
}

void TlmLinearChan::Run_handler(NATIVE_INT_TYPE portNum, NATIVE_UINT_TYPE context) {
    // Only write packets if connected
    if (not this->isConnected_PktSend_OutputPort(0)) {
        return;
    }

    // go through each entry and send a packet if it has been updated
    Fw::TlmPacket pkt;
    pkt.resetPktSer();

    for (U32 entry = 0; entry < TLMCHAN_HASH_BUCKETS; entry++)
    {
        TlmEntry p_entry = this->m_tlmEntries[entry];
        if(p_entry.used && p_entry.updated)
        {
            Fw::SerializeStatus stat = pkt.addValue(p_entry.id, p_entry.lastUpdate, p_entry.buffer);

            // check to see if this packet is full, if so, send it
            if (Fw::FW_SERIALIZE_NO_ROOM_LEFT == stat) {
                this->PktSend_out(0, pkt.getBuffer(), 0);
                // reset packet for more entries
                pkt.resetPktSer();
                // add entry to new packet
                Fw::SerializeStatus stat = pkt.addValue(p_entry.id, p_entry.lastUpdate, p_entry.buffer);
                // if this doesn't work, that means packet isn't big enough for
                // even one channel, so assert
                FW_ASSERT(Fw::FW_SERIALIZE_OK == stat, static_cast<NATIVE_INT_TYPE>(stat));
            } else if (Fw::FW_SERIALIZE_OK == stat) {
                // if there was still room, do nothing move on to the next channel in the packet
            } else  // any other status is an assert, since it shouldn't happen
            {
                FW_ASSERT(0, static_cast<NATIVE_INT_TYPE>(stat));
            }
        }
    }

    // lock mutex long enough to modify active telemetry buffer
    // so the data can be read without worrying about updates
    this->lock();
    // set activeBuffer to not updated
    for (U32 entry = 0; entry < TLMCHAN_HASH_BUCKETS; entry++) {
        this->m_tlmEntries[entry].updated = false;
    }
    this->unLock();

    // send remnant entries
    if (pkt.getNumEntries() > 0) {
        this->PktSend_out(0, pkt.getBuffer(), 0);
    }
}  // end run handler

}  // namespace TlmLinearChan
