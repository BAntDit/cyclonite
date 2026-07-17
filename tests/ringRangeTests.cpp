//
// Created by anton on 6/24/25.
//

#include <core/ringBuffer.h>
#include <gtest/gtest.h>

TEST(RingRangeTest, InitialState)
{
    auto ringRange = cyclonite::core::RingRange<10>{};

    EXPECT_EQ(ringRange.readableSize(), 0);
    EXPECT_EQ(ringRange.freeSize(), 10);
    EXPECT_TRUE(ringRange.empty());
}

TEST(RingRangeTest, BasicReservation)
{
    auto ringRange = cyclonite::core::RingRange<10>{};

    auto offset = ringRange.reserveRange(3);
    EXPECT_NE(offset, cyclonite::core::RingRange<10>::invalid_offset_v);

    EXPECT_EQ(ringRange.readableSize(), 3);
    EXPECT_EQ(ringRange.freeSize(), 7);
    EXPECT_FALSE(ringRange.empty());
}

TEST(RingRangeTest, ContiguousFreeSpace)
{
    auto ringRange = cyclonite::core::RingRange<10>{};

    EXPECT_EQ(ringRange.contiguousFreeSize(), 10);

    ringRange.reserveRange(3);
    EXPECT_EQ(ringRange.contiguousFreeSize(), 7);

    ringRange.reserveRange(7);
    EXPECT_EQ(ringRange.contiguousFreeSize(), 0);
}

TEST(RingRangeTest, WrapAroundBehavior)
{
    auto ringRange = cyclonite::core::RingRange<10>{};

    // Fill whole range completely
    ringRange.reserveRange(10);

    // Free some space by popping
    auto [offset, size] = ringRange.popRange();
    EXPECT_EQ(size, 10);

    // Now we should have space at the beginning
    EXPECT_EQ(ringRange.contiguousFreeSize(), 10);

    ringRange.reserveRange(4);
    EXPECT_EQ(ringRange.contiguousFreeSize(), 6);
}

TEST(RingRangeTest, ExpectedOffsetCalculation)
{
    auto ringRange = cyclonite::core::RingRange<10>{};

    {
        // Simple case - empty buffer
        auto [offset, available] = ringRange.expectedOffset(5);
        EXPECT_EQ(offset, 0);
        EXPECT_EQ(available, 10);
    }

    // After reserving some space
    {
        ringRange.reserveRange(3);
        auto [offset, available] = ringRange.expectedOffset(4);
        EXPECT_EQ(offset, 3);
        EXPECT_EQ(available, 7);
    }

    // Wrap-around case
    {
        ringRange.reserveRange(7); // Now at position 0 again
        auto [offset, available] = ringRange.expectedOffset(3);
        EXPECT_EQ(offset, cyclonite::core::RingRange<10>::invalid_offset_v);
        EXPECT_EQ(available, 0);
    }
}

TEST(RingRangeTest, ForceShiftToBegin)
{
    auto ringRange = cyclonite::core::RingRange<10>{};

    // Fill partially
    ringRange.reserveRange(3);
    ringRange.reserveRange(6);

    // pop 3
    ringRange.popRange();

    // Check expected offset without forcing
    {
        auto [offset, available] = ringRange.expectedOffset(2);
        EXPECT_EQ(offset, 0);
        EXPECT_EQ(available, 3);
    }

    // Check with forceShiftToBegin
    {
        auto [offset, available] = ringRange.expectedOffset(1, true);
        EXPECT_EQ(offset, 0);
        EXPECT_EQ(available, 3);
    }
}

TEST(RingRangeTest, PopRangeBehavior)
{
    auto ringRange = cyclonite::core::RingRange<10>{};

    // Reserve multiple ranges
    ringRange.reserveRange(3);
    ringRange.reserveRange(4);

    // first in, first out
    {
        auto [offset, size] = ringRange.popRange();
        EXPECT_EQ(offset, 0);
        EXPECT_EQ(size, 3);
    }

    // Next pop should get the second reservation
    {
        auto [offset, size] = ringRange.popRange();
        EXPECT_EQ(offset, 3);
        EXPECT_EQ(size, 4);
    }

    // nothing to pop
    EXPECT_TRUE(ringRange.empty());
}

TEST(RingRangeTest, FullBufferHandling)
{
    auto ringRange = cyclonite::core::RingRange<5>{};

    // Fill completely
    EXPECT_NE(ringRange.reserveRange(5), cyclonite::core::RingRange<5>::invalid_offset_v);

    // Try to reserve more
    EXPECT_EQ(ringRange.reserveRange(1), cyclonite::core::RingRange<5>::invalid_offset_v);

    // Free some space
    ringRange.popRange();

    // Now we can reserve again
    EXPECT_NE(ringRange.reserveRange(1), cyclonite::core::RingRange<5>::invalid_offset_v);
}

TEST(RingRangeTest, EdgeCaseSizes)
{
    auto ringRange = cyclonite::core::RingRange<10>{};

    // Reserve exactly the buffer size
    EXPECT_NE(ringRange.reserveRange(10), cyclonite::core::RingRange<10>::invalid_offset_v);
    EXPECT_EQ(ringRange.freeSize(), 0);

    // Try to reserve zero bytes
    EXPECT_EQ(ringRange.reserveRange(0), cyclonite::core::RingRange<10>::invalid_offset_v);

    // Try to reserve more than buffer size
    EXPECT_EQ(ringRange.reserveRange(11), cyclonite::core::RingRange<10>::invalid_offset_v);
}

TEST(RingRangeTest, MultipleReservePopCycles)
{
    auto ringRange = cyclonite::core::RingRange<8>{};

    // Cycle 1
    ringRange.reserveRange(3);
    ringRange.reserveRange(2);
    EXPECT_EQ(ringRange.readableSize(), 5);

    ringRange.popRange();
    EXPECT_EQ(ringRange.readableSize(), 2);

    // Cycle 2 - should wrap around
    ringRange.reserveRange(3);
    EXPECT_EQ(ringRange.readableSize(), 5);

    // Check positions
    auto [offset, size] = ringRange.expectedOffset(1);
    EXPECT_EQ(offset, 0);
    EXPECT_EQ(size, 3);

    // Cycle 3
    ringRange.popRange();
    ringRange.popRange();
    EXPECT_EQ(ringRange.readableSize(), 0);

    ringRange.reserveRange(5);
    EXPECT_EQ(ringRange.readableSize(), 5);
}
