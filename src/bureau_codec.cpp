#include "../include/bureau_codec.h"

#include <cstdint>
#include <array>
#include <span>
#include <cstring>
#include <limits>
#include <stdexcept>

namespace {

    inline void put_u64(std::span<uint8_t> dst, uint64_t v) {
        
        for(int i = 0; i < 8; ++i) {
            
            dst[i] = uint8_t(v >> (8 * i));
        }
    }

    inline void put_u32(std::span<uint8_t> dst, uint32_t v) {
        
        for(int i = 0; i < 4; ++i) {
            
            dst[i] = int8_t(v >> (8 * i));
        }
    }

    inline void put_f32(std::span<uint8_t> dst, float f) {
        
        uint32_t v; 
        std::memcpy(&v,&f,4); 
        put_u32(dst,v);
    }

    inline uint64_t get_u64(std::span<const uint8_t> s) {
        
        uint64_t v = 0; 
        
        for(int i = 0; i < 8; ++i) {
            
            v |= uint64_t(s[i]) << (8*i);
        }
        
        return v; 
    }

    inline uint32_t get_u32(std::span<const uint8_t> s) {
        
        uint32_t v = 0; 
        
        for(int i = 0; i < 4; ++i) {
            
            v |= uint32_t(s[i]) << (8 * i);
        } 
        
        return v; 
    }

    inline float get_f32(std::span<const uint8_t> s) {
        
        uint32_t v = get_u32(s);
        float f;
        std::memcpy(&f,&v,4);
        
        return f; 
    }
    
}

void BureauCodec::encode(const Bureau& b, std::span<uint8_t, kSize> out) {

    if constexpr(sizeof(size_t) < 8) {
        
        if(b.prog_qty > std::numeric_limits<uint64_t>::max()) {
            
            throw std::overflow_error("too big");
        } 
    }

    put_u64(out.subspan<0,8>(), static_cast<uint64_t>(b.prog_qty));
    put_u32(out.subspan<8,4>(), b.math_qty);
    out[12] = b.head_qty; 
    out[13] = out[14] = out[15]=0;
    put_f32(out.subspan<16,4>(), b.salary_sum);
}

Bureau BureauCodec::decode(std::span<const uint8_t, kSize> in){

    Bureau b{};
    uint64_t pq = get_u64(in.subspan<0,8>());

    if constexpr(sizeof(size_t) < 8) { 
        
        if(pq>std::numeric_limits<size_t>::max()) {
            
            throw std::overflow_error("overflow"); 
        }
    }

    b.prog_qty = static_cast<size_t>(pq);
    b.math_qty = get_u32(in.subspan<8,4>());
    b.head_qty = in[12];
    b.salary_sum = get_f32(in.subspan<16,4>());
    
    return b;
}
