#include "../include/mram_mr25h40.h"

#include <cstdint>
#include <span>
#include <array>
#include <vector>
#include <stdexcept>

MR25H40::MR25H40(Spi& spi) : spi_(spi) {}

void MR25H40::power_up_delay() { 
    
    spi_.delay_us(400); 
}

void MR25H40::read(uint32_t addr, std::span<uint8_t> out) {
    
    check_range(addr, out.size());
    std::array<uint8_t,4> hdr{READ, uint8_t(addr>>16), uint8_t(addr>>8), uint8_t(addr)};
    CsGuard cs(spi_);
    spi_.transfer(hdr, {});
    std::vector<uint8_t> zeros(out.size(), 0);
    spi_.transfer(zeros, out);
}

void MR25H40::write(uint32_t addr, std::span<const uint8_t> in) {

    check_range(addr, in.size());
    write_enable();
    std::array<uint8_t,4> hdr{WRITE, uint8_t(addr>>16), uint8_t(addr>>8), uint8_t(addr)};
    CsGuard cs(spi_);
    spi_.transfer(hdr, {});
    spi_.transfer(in, {});
}

uint8_t MR25H40::read_status() {

    uint8_t cmd = RDSR, sr = 0; CsGuard cs(spi_);
    spi_.transfer(std::span{&cmd,1}, {});
    spi_.transfer({}, std::span{&sr,1});
    return sr;
}

void MR25H40::write_status(uint8_t sr) {

    write_enable();
    uint8_t tx[2] = {WRSR, sr}; CsGuard cs(spi_);
    spi_.transfer(tx, {});
}

void MR25H40::write_enable() { 

    uint8_t c = WREN; 
    CsGuard cs(spi_); 
    spi_.transfer(std::span{&c,1}, {}); 
}

void MR25H40::write_disable() { 
    
    uint8_t c = WRDI; 
    CsGuard cs(spi_); 
    spi_.transfer(std::span{&c,1}, {}); 
}

void MR25H40::sleep() { 
    
    uint8_t c = SLP; 
    
    { 
        CsGuard cs(spi_); 
        spi_.transfer(std::span{&c,1}, {});
    } 
    
    spi_.delay_us(3); }

void MR25H40::wake() {  
    
    uint8_t c=WAK; 
    
    { 
        CsGuard cs(spi_); 
        spi_.transfer(std::span{&c,1}, {});
    } 
    
    spi_.delay_us(400); 
}

void MR25H40::set_block_protect(Protect p, bool hw_lock) {

    uint8_t sr = read_status() & ~(0x0E);

    if (p == Protect::UpperQuarter) {
        
        sr |= 1 << 2;
    } else if (p == Protect::UpperHalf) {
        
        sr |= 1 << 3;
    } else if (p == Protect::All) {
        
        sr |= (1 << 2) | (1 << 3);
    }

    if (hw_lock){ 
        
        spi_.set_wp(false); 
        sr |= (1 << 7); 
    }

    write_status(sr);
}

void MR25H40::check_range(uint32_t addr, size_t len) {

    if (addr >= kSize || len > (kSize - addr)) {
        
        throw std::out_of_range("MR25H40: range");
    }
}
