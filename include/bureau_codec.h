#pragma once

#include <cstdint>
#include <array>
#include <span>
#include <cstring>
#include <limits>
#include <stdexcept>

struct Bureau {
    size_t   prog_qty;
    uint32_t math_qty;
    uint8_t  head_qty;
    float    salary_sum;
};

namespace BureauCodec {

    constexpr size_t kSize = 20;
    constexpr uint16_t kVersion = 1;

    void encode(const Bureau& b, std::span<uint8_t, kSize> out);
    Bureau decode(std::span<const uint8_t, kSize> in);

}//ns_bureau_codec
