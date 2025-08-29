#pragma once

#include <optional>
#include <array>
#include <cstring>

#include "mram_mr25h40.h"
#include "bureau_codec.h"

struct RecordHeader {
    uint32_t magic;
    uint16_t version;
    uint16_t reserved;
    uint32_t length;
    uint32_t crc32;
    uint64_t seqno;
};

static_assert(sizeof(RecordHeader)==24);

class BureauStore {

public:
    explicit BureauStore(MR25H40& mram);
    void write(const Bureau& b);
    Bureau read();

private:
    MR25H40& mram_;
    static constexpr uint32_t MAGIC = 0x45525542; //=BURE
    static constexpr uint32_t SLOT_SZ = 256;
    static constexpr uint32_t SLOT_A = 0;
    static constexpr uint32_t SLOT_B = SLOT_A + SLOT_SZ;

    struct Pick { std::optional<RecordHeader> header; uint32_t base=0; };
    std::optional<RecordHeader> read_hdr(uint32_t base);
    static bool choose_slot_for_write(const std::optional<RecordHeader>& a,const std::optional<RecordHeader>& b);
    Pick pick_best();

};//class_bureau_store
