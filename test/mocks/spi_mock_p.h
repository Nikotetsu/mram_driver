#pragma once

#include <vector>
#include <stdexcept>
#include <cstdint>
#include <span>
#include <array>

#include "../../include/spi.h"
#include "../../include/mram_mr25h40.h"

class SpiMockP : public Spi {

    std::vector<uint8_t> mem;
    bool cs_low = false;
    std::vector<uint8_t> inbuf;
    uint8_t sr = 0;

    uint32_t start_protected() const {
        // Определяем начало защищённой области по BP0/BP1
        bool bp0 = sr & MR25H40::SR_BP0;
        bool bp1 = sr & MR25H40::SR_BP1;

        if(!bp0 && !bp1) return MR25H40::kSize;       // None (ничего не защищено)
        if(bp0 && !bp1) return 0x60000;               // Upper Quarter (последние 128К)
        if(!bp0 && bp1) return 0x40000;               // Upper Half (последние 256К)
        if(bp0 && bp1)  return 0x00000;               // All
        
        return MR25H40::kSize;
    }

public:

    SpiMockP(): mem(MR25H40::kSize,0) {}

    void transfer(std::span<const uint8_t> tx, std::span<uint8_t> rx) override {
        
        if(!cs_low) {
            
            throw std::runtime_error("CS high during transfer");
        }
        // Накопим переданные данные
        inbuf.insert(inbuf.end(), tx.begin(), tx.end());
        
        // Если это команда чтения регистра статуса
        if(!inbuf.empty() && inbuf[0] == MR25H40::RDSR && !rx.empty()){

            rx[0] = sr;
            return;
        }

        // Если это READ (0x03)
        if(inbuf.size() >= 4 && inbuf[0] == MR25H40::READ){

            uint32_t addr = (uint32_t(inbuf[1]) << 16) | (uint32_t(inbuf[2]) << 8) | inbuf[3];
            
            for(size_t i = 0; i < rx.size(); ++i) {

                if(addr + i < mem.size()) {
                    
                    rx[i] = mem[addr+i];
                } else {
                    
                    rx[i] = 0xFF;
                }
            }
        }
    }

    void cs_assert() override { 

        cs_low = true; 
        inbuf.clear(); 
    }

    void cs_deassert() override {
        
        if(!inbuf.empty()){

            uint8_t cmd = inbuf[0];

            switch(cmd){

                case MR25H40::WREN: {
                    sr |= MR25H40::SR_WEL; 
                    break;
                }
                case MR25H40::WRDI: {
                    sr &= ~MR25H40::SR_WEL; 
                    break;
                }
                case MR25H40::WRSR: {
                    if(sr & MR25H40::SR_WEL && inbuf.size() >= 2) {

                        sr = inbuf[1]; // очень упрощённо
                        sr &= (MR25H40::SR_BP0 | MR25H40::SR_BP1 | MR25H40::SR_WEL | MR25H40::SR_WD);
                        sr &= ~MR25H40::SR_WEL; // WEL сбрасывается после записи
                    }
                    break;
                }
                case MR25H40::WRITE: {
                    if(sr & MR25H40::SR_WEL && inbuf.size() >= 4){

                        uint32_t addr = (uint32_t(inbuf[1]) << 16) | (uint32_t(inbuf[2]) << 8) | inbuf[3];
                        uint32_t prot = start_protected();

                        if(addr >= prot){
                            // Запрещённая область — игнорируем запись
                        } else {

                            for(size_t i = 4; i < inbuf.size(); ++i){

                                if(addr + i - 4< mem.size()) {

                                    mem[addr+i-4]=inbuf[i];
                                }
                            }
                        }

                        sr &= ~MR25H40::SR_WEL; // WEL сбрасывается после записи
                    }
                    break;
                }
                default: break;
            }
        }

        cs_low = false; 
        inbuf.clear();
    }

    void delay_us(uint32_t) override {}
};
