#include <gtest/gtest.h>
#include <cstdint>
#include <span>
#include <array>

#include "../../include/bureau_store.h"
#include "../../include/mram_mr25h40.h"
#include "../mocks/spi_mock_p.h"   // persistent memory mock

// The real BureauStore API (per implementation) is:
//   void write(const Bureau&);
//   Bureau read();
//
// Tests below match that exact API.

static Bureau make_bureau(size_t prog, uint32_t math, uint8_t head, float salary) {
    Bureau b{};
    b.prog_qty = prog;
    b.math_qty = math;
    b.head_qty = head;
    b.salary_sum = salary;
    return b;
}

TEST(BureauStore, WriteThenReadSingle) {
    SpiMockP spi;
    MR25H40 mram(spi);
    BureauStore store(mram);

    Bureau b = make_bureau(1, 10, 2, 123.0f);
    ASSERT_NO_THROW(store.write(b));

    Bureau r = store.read();
    EXPECT_EQ(r.prog_qty, 1u);
    EXPECT_EQ(r.math_qty, 10u);
    EXPECT_EQ(r.head_qty, 2u);
    EXPECT_FLOAT_EQ(r.salary_sum, 123.0f);
}

TEST(BureauStore, MultipleWritesReturnLatest) {
    SpiMockP spi;
    MR25H40 mram(spi);
    BureauStore store(mram);

    store.write(make_bureau(1, 10, 2, 11.0f));
    store.write(make_bureau(2, 20, 3, 22.0f));
    store.write(make_bureau(3, 30, 4, 33.0f));

    Bureau r = store.read();
    EXPECT_EQ(r.prog_qty, 3u);
    EXPECT_EQ(r.math_qty, 30u);
    EXPECT_EQ(r.head_qty, 4u);
    EXPECT_FLOAT_EQ(r.salary_sum, 33.0f);
}

TEST(BureauStore, PersistenceWithinSameSpiMockInstance) {
    SpiMockP spi;
    MR25H40 mram(spi);

    {
        BureauStore store(mram);
        store.write(make_bureau(7, 70, 5, 77.0f));
    }

    // Recreate the store using the same MRAM/SPI mock; data should be readable
    {
        BureauStore store2(mram);
        Bureau r = store2.read();
        EXPECT_EQ(r.prog_qty, 7u);
        EXPECT_EQ(r.math_qty, 70u);
        EXPECT_EQ(r.head_qty, 5u);
        EXPECT_FLOAT_EQ(r.salary_sum, 77.0f);
    }
}
