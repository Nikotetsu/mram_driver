#include <gtest/gtest.h>
#include <array>
#include <cstdint>
#include <span>

#include "../../include/bureau_codec.h"

TEST(BureauCodec, EncodeDecodeRoundTrip) {
    Bureau b{};
    b.prog_qty = 123456u;
    b.math_qty = 654321u;
    b.head_qty = 42u;
    b.salary_sum = 9876.5f;

    std::array<uint8_t, BureauCodec::kSize> buf{};
    BureauCodec::encode(b, std::span<uint8_t, BureauCodec::kSize>(buf));

    Bureau restored = BureauCodec::decode(std::span<const uint8_t, BureauCodec::kSize>(buf));

    EXPECT_EQ(b.prog_qty, restored.prog_qty);
    EXPECT_EQ(b.math_qty, restored.math_qty);
    EXPECT_EQ(b.head_qty, restored.head_qty);
    EXPECT_FLOAT_EQ(b.salary_sum, restored.salary_sum);
}
