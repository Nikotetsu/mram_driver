#pragma once

#include <cstdint>
#include <span>

// Абстрактный SPI интерфейс
struct Spi {

    virtual void transfer(std::span<const uint8_t> tx, std::span<uint8_t> rx) = 0;
    virtual void cs_assert() = 0;
    virtual void cs_deassert() = 0;
    virtual void delay_us(uint32_t us) = 0;
    virtual void set_wp(bool high) {}
    virtual void set_hold(bool high) {}
    virtual ~Spi() = default;

};//class_abstract_spi

// RAII-обертка для CS
struct CsGuard {

    Spi& s;
    explicit CsGuard(Spi& s_) : s(s_) { s.cs_assert(); }
    ~CsGuard() { s.cs_deassert(); }

};//class_cs_protection_wrapper
