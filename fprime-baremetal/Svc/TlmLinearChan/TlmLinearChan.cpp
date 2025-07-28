// ======================================================================
// \title  TlmLinearChan.cpp
// \author ethanchee
// \brief  Implementation file for channelized telemetry storage component
// ======================================================================

#include <config/FpConfig.hpp>
#include <Fw/Com/ComBuffer.hpp>
#include <Fw/Types/Assert.hpp>
#include <fprime-baremetal/Svc/TlmLinearChan/TlmLinearChan.hpp>

#include <new>

namespace Baremetal {

TlmLinearChan::TlmLinearChan(const char* name) : TlmLinearChanComponentBase(name), m_setupDone(false) {}

TlmLinearChan::~TlmLinearChan()
{
    if (this->m_tlmEntries != nullptr)
    {
        // First destruct the TLM entry structs
        for (auto i = 0; i < TLMCHAN_HASH_BUCKETS; i++)
        {
            this->m_tlmEntries[i].~TlmEntry();
        }
        // Then deallocate the memory
        this->m_allocator->deallocate(this->m_memId, this->m_tlmEntries);
    }
}

void TlmLinearChan::init(FwSizeType queueDepth, /*!< The queue depth*/
                         FwEnumStoreType instance    /*!< The instance number*/
) {
    TlmLinearChanComponentBase::init(queueDepth, instance);
}

void TlmLinearChan::setup(FwEnumStoreType memId, Fw::MemAllocator& allocator)
{
    FW_ASSERT(!this->m_setupDone);
    this->m_allocator = &allocator;
    this->m_memId = memId;

    // Allocate memory for TLM entries
    FwSizeType expected_size = TLMCHAN_HASH_BUCKETS * sizeof(TlmEntry);
    FwSizeType allocated_size = expected_size;
    bool recoverable = false;
    void* memory = allocator.allocate(memId, allocated_size, recoverable);
    FW_ASSERT(memory != nullptr && allocated_size == expected_size, allocated_size);
    this->m_tlmEntries = static_cast<TlmEntry*>(memory);
    // Initialize TLM entries
    for (auto i = 0; i < TLMCHAN_HASH_BUCKETS; i++)
    {
        void* address = static_cast<void*>(this->m_tlmEntries + i);
        new (address) TlmEntry();
    }

    // clear buckets
    for (U32 entry = 0; entry < TLMCHAN_HASH_BUCKETS; entry++) {
        this->m_tlmEntries[entry].updated = false;
        this->m_tlmEntries[entry].id = 0;
        this->m_tlmEntries[entry].used = false;
    }

    this->m_setupDone = true;
}

void TlmLinearChan::pingIn_handler(const FwIndexType portNum, U32 key) {
    // return key
    this->pingOut_out(0, key);
}

Fw::TlmValid TlmLinearChan::TlmGet_handler(FwIndexType portNum, FwChanIdType id, Fw::Time& timeTag, Fw::TlmBuffer& val) {
    FW_ASSERT(this->m_setupDone);

    // Compute index for entry
    U32 entry;
    for (entry = 0; entry < TLMCHAN_HASH_BUCKETS; entry++) {
        if (this->m_tlmEntries[entry].id == id) {  // If bucket exists, check id
            break;
        }
    }

    if(entry < TLMCHAN_HASH_BUCKETS) {
        val = this->m_tlmEntries[entry].buffer;
        timeTag = this->m_tlmEntries[entry].lastUpdate;
        return Fw::TlmValid::VALID;
    } else {  // requested entry may not be written yet; empty buffer
        val.resetSer();
        return Fw::TlmValid::INVALID;
    }
}

void TlmLinearChan::TlmRecv_handler(FwIndexType portNum, FwChanIdType id, Fw::Time& timeTag, Fw::TlmBuffer& val) {
    FW_ASSERT(this->m_setupDone);

    // Compute index for entry
    U32 entry;
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

void TlmLinearChan::Run_handler(FwIndexType portNum, U32 context) {
    // Only write packets if connected
    if (not this->isConnected_PktSend_OutputPort(0)) {
        return;
    }
    FW_ASSERT(this->m_setupDone);

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
                FW_ASSERT(Fw::FW_SERIALIZE_OK == stat, static_cast<FwAssertArgType>(stat));
            } else if (Fw::FW_SERIALIZE_OK == stat) {
                // if there was still room, do nothing move on to the next channel in the packet
            } else  // any other status is an assert, since it shouldn't happen
            {
                FW_ASSERT(0, static_cast<FwAssertArgType>(stat));
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
