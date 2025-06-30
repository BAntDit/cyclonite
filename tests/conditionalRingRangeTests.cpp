//
// Created by anton on 6/30/25.
//

//
// Created by anton on 6/24/25.
//

#include <gtest/gtest.h>
#include <core/ringBuffer.h>
#include <cstdint>

TEST(ConditionalRingRangeTest, BasicReservation) {
    auto ringRange = cyclonite::core::ConditionalRingRange<10, uint32_t>{};

    auto offset = ringRange.reserveRange(100, 3);
    EXPECT_NE(offset, cyclonite::core::RingRange<10>::invalid_offset_v);

    EXPECT_EQ(ringRange.readableSize(), 3);
    EXPECT_EQ(ringRange.freeSize(), 7);
    EXPECT_FALSE(ringRange.empty());
}

TEST(ConditionalRingRangeTest, PopRangeBehavior) {
    auto ringRange = cyclonite::core::ConditionalRingRange<10, uint32_t>{};

    // Reserve multiple ranges
    ringRange.reserveRange(100, 3);
    ringRange.reserveRange(101, 4);

    // first in, first out
    {
        auto [offset, size] = ringRange.popRange([](uint32_t v) -> bool {
            return v <=  100;
        });
        EXPECT_EQ(offset, 0);
        EXPECT_EQ(size, 3);
    }

    // Next pop should get the second reservation
    {
        auto [offset, size] = ringRange.popRange([](auto v) -> bool {
            return v <= 101;
        });
        EXPECT_EQ(offset, 3);
        EXPECT_EQ(size, 4);
    }

    // nothing to pop
    EXPECT_TRUE(ringRange.empty());
}

TEST(ConditionalRingRangeTest, MultipleReservePopCycles) {
    auto ringRange = cyclonite::core::ConditionalRingRange<8, uint32_t>{};

    // Cycle 1
    ringRange.reserveRange(100, 3);
    ringRange.reserveRange(101, 2);
    EXPECT_EQ(ringRange.readableSize(), 5);

    ringRange.popRange([](uint32_t v) -> bool { return v > 100; });
    EXPECT_EQ(ringRange.readableSize(), 5);

    // Cycle 2 - should wrap around
    ringRange.reserveRange(102, 3);
    EXPECT_EQ(ringRange.readableSize(), 8);

    // Check positions
    auto [offset, size] = ringRange.expectedOffset(1);
    EXPECT_EQ(offset, cyclonite::core::RingRange<8>::invalid_offset_v);
    EXPECT_EQ(size, 0);

    // Cycle 3
    ringRange.popRange([](uint32_t v) -> bool { return v > 99; });
    ringRange.popRange([](uint32_t v) -> bool { return v > 99; });
    EXPECT_EQ(ringRange.readableSize(), 3);

    ringRange.reserveRange(103, 5);
    EXPECT_EQ(ringRange.readableSize(), 8);
}
