#include <gtest/gtest.h>
#include <vector>
#include <array>

#include "../../include/bureau_store.h"
#include "../../include/mram_mr25h40.h"
#include "../mocks/spi_mock_p.h"

// End-to-end: encode -> store -> read back through the public APIs only.
TEST(E2E, FullFlowThreeWritesLatestWins) {
    SpiMockP spi;           // persistent memory across the driver lifetime
    MR25H40 mram(spi);
    BureauStore store(mram);

    Bureau a{.prog_qty=10, .math_qty=1, .head_qty=1, .salary_sum=10.0f};
    Bureau b{.prog_qty=20, .math_qty=2, .head_qty=2, .salary_sum=20.0f};
    Bureau c{.prog_qty=30, .math_qty=3, .head_qty=3, .salary_sum=30.0f};

    store.write(a);
    store.write(b);
    store.write(c);

    Bureau r = store.read();
    EXPECT_EQ(r.prog_qty, 30u);
    EXPECT_EQ(r.math_qty, 3u);
    EXPECT_EQ(r.head_qty, 3u);
    EXPECT_FLOAT_EQ(r.salary_sum, 30.0f);
}
