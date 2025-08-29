#include <gtest/gtest.h>
#include <vector>
#include <array>
#include <cstdint>
#include <span>

#include "../../include/mram_mr25h40.h"
#include "../mocks/spi_mock.h"   // non-persistent mock

// --- MR25H40 driver tests that match the actual API (write/read, read_status returning uint8_t) ---

TEST(MRAM_ReadWrite, RoundTripSmall) {
    SpiMock spi;
    MR25H40 mram(spi);

    const uint32_t addr = 0x100;
    std::array<uint8_t, 64> in{};
    for (size_t i = 0; i < in.size(); ++i) in[i] = static_cast<uint8_t>(i * 3);

    EXPECT_NO_THROW(mram.write(addr, std::span<const uint8_t>(in.data(), in.size())));

    std::array<uint8_t, 64> out{};
    EXPECT_NO_THROW(mram.read(addr, std::span<uint8_t>(out.data(), out.size())));
    EXPECT_EQ(in, out);
}

TEST(MRAM_ReadWrite, RoundTripCrossBoundary) {
    SpiMock spi;
    MR25H40 mram(spi);

    // Write/read a chunk that likely crosses internal page boundaries
    const uint32_t addr = 0x1F0; // unaligned
    std::vector<uint8_t> in(513);
    for (size_t i = 0; i < in.size(); ++i) in[i] = static_cast<uint8_t>((i * 7) & 0xFF);

    ASSERT_NO_THROW(mram.write(addr, std::span<const uint8_t>(in.data(), in.size())));

    std::vector<uint8_t> out(in.size());
    ASSERT_NO_THROW(mram.read(addr, std::span<uint8_t>(out.data(), out.size())));
    EXPECT_EQ(in, out);
}

TEST(MRAM_RangeCheck, ThrowsOnOutOfRangeReadWrite) {
    SpiMock spi;
    MR25H40 mram(spi);

    // Write that goes beyond device size must throw
    std::vector<uint8_t> big(1024);
    EXPECT_THROW(mram.write(MR25H40::kSize - 100, std::span<const uint8_t>(big.data(), big.size())), std::out_of_range);

    // Read that goes beyond device size must throw
    std::vector<uint8_t> out(500);
    EXPECT_THROW(mram.read(MR25H40::kSize - 200, std::span<uint8_t>(out.data(), out.size())), std::out_of_range);
}

TEST(MRAM_Status, WriteEnableDisableBit) {
    SpiMock spi;
    MR25H40 mram(spi);

    mram.write_enable();
    uint8_t sr = mram.read_status();
    EXPECT_NE((sr & MR25H40::SR_WEL), 0u) << "WEL bit should be set after write_enable()";

    mram.write_disable();
    sr = mram.read_status();
    EXPECT_EQ((sr & MR25H40::SR_WEL), 0u) << "WEL bit should be cleared after write_disable()";
}

TEST(MRAM_Power, SleepWakeAffectsMock) {
    SpiMock spi;
    MR25H40 mram(spi);

    // In SpiMock there is a 'sleeping' flag toggled by sleep()/wake().
    mram.sleep();
    // We can check mock state directly to make sure calls were wired.
    EXPECT_TRUE(spi.sleeping);

    mram.wake();
    EXPECT_FALSE(spi.sleeping);
}

TEST(MRAM_Protection, SRWDBitSetOnHwLock) {
    SpiMock spi;
    MR25H40 mram(spi);

    // Hardware lock should set SRWD (bit 7) inside status register
    mram.set_block_protect(MR25H40::Protect::All, true);
    uint8_t sr = mram.read_status();
    EXPECT_NE((sr & MR25H40::SR_WD), 0u) << "SRWD bit must be set after hw-locked protection";
}
