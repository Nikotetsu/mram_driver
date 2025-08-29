#pragma once

#include <cstdint>
#include <span>
#include <array>
#include <vector>
#include <stdexcept>

#include "spi.h"

class MR25H40 {

public:
    static constexpr uint32_t kSize = 512 * 1024; //512Kib
    static constexpr uint8_t WREN = 0x06, WRDI = 0x04;
    static constexpr uint8_t RDSR = 0x05, WRSR = 0x01;
    static constexpr uint8_t READ = 0x03, WRITE = 0x02;
    static constexpr uint8_t SLP = 0xB9, WAK = 0xAB;
    static constexpr uint8_t SR_WEL = 0x02;   // Bit 1: Write Enable Latch
    static constexpr uint8_t SR_BP0 = 0x04;   // Bit 2: Block Protect 0
    static constexpr uint8_t SR_BP1 = 0x08;   // Bit 3: Block Protect 1
    static constexpr uint8_t SR_WD = 0x80;    // Bit 7: Status Register Write Disable (SRWD)

    explicit MR25H40(Spi& spi);

    void power_up_delay();
    void read(uint32_t addr, std::span<uint8_t> out);
    void write(uint32_t addr, std::span<const uint8_t> in);
    uint8_t read_status();
    void write_status(uint8_t sr);
    void write_enable();
    void write_disable();
    void sleep();
    void wake();

    enum class Protect { None, UpperQuarter, UpperHalf, All };
    void set_block_protect(Protect p, bool hw_lock = false);

private:
    Spi& spi_;
    static void check_range(uint32_t addr, size_t len);

};//class_mr25h40
