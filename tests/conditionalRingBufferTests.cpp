//
// Created by anton on 7/7/25.
//

#include <core/ringBuffer.h>
#include <cstdint>
#include <gtest/gtest.h>

TEST(ConditionalRingBufferTest, InitialState)
{
    auto buffer = cyclonite::core::ConditionalRingBuffer<std::byte, uint32_t, 1024>{};

    EXPECT_TRUE(buffer.empty());
    EXPECT_EQ(buffer.readableSize(), 0);
    EXPECT_EQ(buffer.freeSize(), 1024);
}

TEST(ConditionalRingBufferTest, PopBufferBehavior)
{
    auto buffer = cyclonite::core::ConditionalRingBuffer<std::byte, uint32_t, 1024>{};

    // Reserve
    buffer.reserveToWrite<uint32_t>(1, 2);
    buffer.reserveToWrite<uint32_t>(2, 2);

    // first in, first out
    {
        auto count = buffer.pop([](auto v) -> bool { return v <= 2; });
        EXPECT_EQ(count, sizeof(uint32_t) * 2);
    }

    // Next pop should get the second reservation
    {
        auto count = buffer.pop([](auto v) -> bool { return v <= 2; });
        EXPECT_EQ(count, sizeof(uint32_t) * 2);
    }

    // nothing to pop
    EXPECT_TRUE(buffer.empty());
}

TEST(ConditionalRingBufferTest, MultipleReservePopCycles)
{
    auto buffer = cyclonite::core::ConditionalRingBuffer<std::byte, uint32_t, 1024>{};

    // Cycle 1
    buffer.reserveToWrite<uint32_t>(100, 3);
    buffer.reserveToWrite<uint32_t>(101, 2);
    EXPECT_GE(buffer.readableSize(), sizeof(uint32_t) * 5);

    buffer.pop([](uint32_t v) -> bool { return v > 100; });
    EXPECT_GE(buffer.readableSize(), sizeof(uint32_t) * 5);

    // Cycle 2 - should wrap around
    buffer.reserveToWrite<uint32_t>(102, 3);
    EXPECT_GE(buffer.readableSize(), sizeof(uint32_t) * 8);

    // Check positions
    auto view = buffer.reserveToWrite<uint32_t>(102, 1000);
    EXPECT_EQ(view.offset(), cyclonite::core::RingRange<8>::invalid_offset_v);
    EXPECT_FALSE(static_cast<bool>(view));

    // Cycle 3
    buffer.pop([](uint32_t v) -> bool { return v > 99; });
    buffer.pop([](uint32_t v) -> bool { return v > 99; });
    EXPECT_LT(buffer.readableSize(), sizeof(uint32_t) * 5);

    buffer.reserveToWrite<uint32_t>(103, 5);
    EXPECT_GE(buffer.readableSize(), sizeof(uint32_t) * 8);
}
