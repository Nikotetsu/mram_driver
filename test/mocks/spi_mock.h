#pragma once

#include <vector>
#include <stdexcept>
#include <cstdint>
#include <span>
#include <array>

#include "../../include/spi.h"
#include "../../include/mram_mr25h40.h"

class SpiMock : public Spi {
public:

    SpiMock() : mem(MR25H40::kSize) {}

    void transfer(std::span<const uint8_t> tx,std::span<uint8_t> rx) override {

        if(!cs_low) {
            
            throw std::runtime_error("CS high");
        }

        for(uint8_t b : tx) {
            
            inbuf.push_back(b);
        }

        if(!rx.empty()){
            
            if(!inbuf.empty() && inbuf[0] == MR25H40::RDSR) {
                
                rx[0] = sr;
                return;
            }

            if(inbuf.size() >= 4 && inbuf[0] == MR25H40::READ) {
                
                uint32_t a = (inbuf[1] << 16) | (inbuf[2] << 8) | inbuf[3];
                
                for(size_t i = 0; i < rx.size(); ++i){
                    
                    rx[i]=mem.at(a+i);
                }
                
                return;
            }

            std::fill(rx.begin(),rx.end(),0);
        }
    }
    void cs_assert() override {
        
        cs_low = true;inbuf.clear();
    }

    void cs_deassert() override {

        if(!inbuf.empty()) {

            uint8_t c = inbuf[0];
            
            switch(c){
                
                case MR25H40::WREN: {
                    sr |= MR25H40::SR_WEL; 
                    break;
                }

                case MR25H40::WRDI: {
                    sr &= ~MR25H40::SR_WEL; 
                    break;
                }

                case MR25H40::WRSR: {
                    if(sr&MR25H40::SR_WEL) {
                        
                        sr=(sr&MR25H40::SR_WEL)|inbuf[1];
                    } 
                    break;
                }

                case MR25H40::WRITE: {
                    if(sr&MR25H40::SR_WEL) {
                        
                        uint32_t a = (inbuf[1] << 16) | (inbuf[2] << 8) | inbuf[3];
                        
                        for(size_t i = 4; i < inbuf.size(); ++i) {
                            
                            mem.at(a + i - 4) = inbuf[i];
                        }
                    } 
                    break;
                }

                case MR25H40::SLP: {
                    sleeping = true; 
                    break; 
                }
                    
                case MR25H40::WAK: {
                    sleeping = false; 
                    break;
                }
            }
        } 
        
        cs_low = false;
    }

    void delay_us(uint32_t) override {}

    std::vector<uint8_t> mem; 
    uint8_t sr = 0; 
    bool cs_low = false; 
    bool sleeping = false; 
    std::vector<uint8_t> inbuf;

};//class_SpiMock
