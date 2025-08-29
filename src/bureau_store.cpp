#include "../include/bureau_store.h"

#include <stdexcept>
#include <vector>
#include <optional>
#include <array>
#include <cstring>

class Crc32 {
public:

    static uint32_t calc(std::span<const uint8_t> data){

        uint32_t c = ~0u; 
        
        for(uint8_t b:data) {
            c ^= b; 
            
            for(int i=0;i<8;++i) {
                
                c = (c&1) ? (0xEDB88320u^(c >> 1)) : (c >> 1);
            }
        } 
        
        return ~c;
    }

};//class_crc32

BureauStore::BureauStore(MR25H40& mram):mram_(mram){}

void BureauStore::write(const Bureau& b) {

    std::array<uint8_t,BureauCodec::kSize> payload{};
    BureauCodec::encode(b,payload);
    const uint32_t crc = Crc32::calc(payload);
    auto ah = read_hdr(SLOT_A), bh = read_hdr(SLOT_B);
    uint64_t next_seq = 1;
    
    if(ah && bh) {
        
        next_seq = std::max(ah->seqno,bh->seqno)+1;
    
    } else if(ah) {
        
        next_seq = ah->seqno+1;
    
    } else if(bh) {
        
        next_seq = bh->seqno+1;
    
    }

    RecordHeader h{MAGIC,BureauCodec::kVersion,0,uint32_t(payload.size()),crc,next_seq};
    const uint32_t base = choose_slot_for_write(ah,bh)?SLOT_A:SLOT_B;
    mram_.write(base + sizeof(RecordHeader),payload);
    std::array<uint8_t,sizeof(RecordHeader) > hb{}; 
    std::memcpy(hb.data(),&h,hb.size());
    mram_.write(base,hb);

}

Bureau BureauStore::read() {

    auto pick = pick_best(); 
    
    if(!pick.header) {
        
        throw std::runtime_error("no record");
    }

    const auto& h = *pick.header; 
    
    if(h.magic != MAGIC || h.version != BureauCodec::kVersion || h.length != BureauCodec::kSize) {
        
        throw std::runtime_error("bad header");
    }

    std::array<uint8_t,BureauCodec::kSize> payload{}; 
    mram_.read(pick.base + sizeof(RecordHeader),payload);
    
    if(Crc32::calc(payload) != h.crc32) {
        
        throw std::runtime_error("CRC mismatch");
    }

    return BureauCodec::decode(payload);
}

std::optional<RecordHeader> BureauStore::read_hdr(uint32_t base) {

    std::array<uint8_t,sizeof(RecordHeader) > hb{}; 
    mram_.read(base,hb);

    RecordHeader h{}; 

    std::memcpy(&h,hb.data(),hb.size());

    if(h.magic != MAGIC || h.version != BureauCodec::kVersion || h.length != BureauCodec::kSize) {
        
        return std::nullopt;
    } 
    
    return h;
}

bool BureauStore::choose_slot_for_write(const std::optional<RecordHeader>& a,const std::optional<RecordHeader>& b) {

    if(a && b) {
        
        return a->seqno <= b->seqno;
    } 
    
    if(a) {
        
        return false;
    } 
    
    return true;
}
BureauStore::Pick BureauStore::pick_best() {

    auto ah = read_hdr(SLOT_A), bh = read_hdr(SLOT_B);

    if(ah && bh) {
        
        return (ah->seqno >= bh->seqno) ? Pick{ah,SLOT_A} : Pick{bh,SLOT_B};
    }

    if(ah) {
        
        return {ah,SLOT_A};
    }
    
    if(bh) {
        
        return {bh,SLOT_B};
    } 
    
    return {};
}
