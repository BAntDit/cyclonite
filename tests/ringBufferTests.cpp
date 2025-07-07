
#include <core/ringBuffer.h>
#include <glm/ext/scalar_uint_sized.hpp>
#include <gtest/gtest.h>

TEST(RingBUfferTest, InitialState)
{
    auto buffer = cyclonite::core::RingBuffer<std::byte, 1024>{};

    EXPECT_TRUE(buffer.empty());
    EXPECT_EQ(buffer.readableSize(), 0);
    EXPECT_EQ(buffer.freeSize(), 1024);
}

TEST(ByteRingBufferTest, ReserveAndPop)
{
    auto buffer = cyclonite::core::RingBuffer<std::byte, 1024>{};
    auto view = buffer.reserveToWrite<int>(2);

    ASSERT_NE(view.data(), nullptr);
    EXPECT_EQ(view.count(), 2);
    EXPECT_EQ(buffer.readableSize(), sizeof(int) * 2);
    EXPECT_EQ(buffer.freeSize(), 1024 - sizeof(int) * 2);

    auto popped = buffer.pop();
    EXPECT_EQ(popped, sizeof(int) * 2);
    EXPECT_TRUE(buffer.empty());
}

TEST(ByteRingBufferTest, WrapAround)
{
    auto buffer = cyclonite::core::RingBuffer<std::byte, 1024>{};

    buffer.reserveToWrite<std::byte>(500);
    buffer.reserveToWrite<std::byte>(500);

    // Pop most of it
    buffer.pop();

    // Now write should wrap around
    auto view = buffer.reserveToWrite<std::byte>(500);
    EXPECT_EQ(view.count(), 500);
    EXPECT_EQ(view.offset(), 0);
}

TEST(TypedRingBufferTest, InitialState)
{
    auto buffer = cyclonite::core::RingBuffer<uint32_t, 100>{};

    EXPECT_TRUE(buffer.empty());
    EXPECT_EQ(buffer.readableSize(), 0);
    EXPECT_EQ(buffer.freeSize(), 100);
}

TEST(TypedRingBufferTest, ReserveAndPop)
{
    auto buffer = cyclonite::core::RingBuffer<uint32_t, 100>{};

    auto view = buffer.reserveToWrite(10);
    ASSERT_NE(view.data(), nullptr);
    EXPECT_EQ(view.count(), 10);
    EXPECT_EQ(buffer.readableSize(), 10);
    EXPECT_EQ(buffer.freeSize(), 90);

    auto popped = buffer.pop();
    EXPECT_EQ(popped.count(), 10);
    EXPECT_TRUE(buffer.empty());
}

TEST(TypedRingBufferTest, ContiguousOperations)
{
    auto buffer = cyclonite::core::RingBuffer<uint32_t, 100>{};

    // First write
    auto view1 = buffer.reserveToWrite(60);
    EXPECT_EQ(view1.count(), 60);

    // Second write
    auto view2 = buffer.reserveToWrite(30);
    EXPECT_EQ(view2.count(), 30);

    // Pop first write
    buffer.pop();

    // Third write should wrap around
    auto view3 = buffer.reserveToWrite(50);
    EXPECT_EQ(view3.count(), 50);
    EXPECT_EQ(view3.offset(), view1.offset());
}

TEST(ExternalByteRingBufferTest, InitialState)
{
    std::vector<std::byte> external_buffer(1024);
    cyclonite::core::RingBuffer<std::byte, 1024, true> buffer{ external_buffer.data() };

    EXPECT_TRUE(buffer.empty());
    EXPECT_EQ(buffer.readableSize(), 0);
    EXPECT_EQ(buffer.freeSize(), 1024);
}

TEST(ExternalByteRingBufferTest, DataAccess)
{
    std::vector<std::byte> external_buffer(1024);
    cyclonite::core::RingBuffer<std::byte, 1024, true> buffer{ external_buffer.data() };

    EXPECT_EQ(buffer.data(), external_buffer.data());

    auto view = buffer.reserveToWrite<int>(1);
    *view.data() = 42;
    EXPECT_EQ(reinterpret_cast<int*>(external_buffer.data())[0], 42);
}

TEST(ExternalTypedRingBufferTest, InitialState)
{
    std::vector<uint32_t> external_buffer(100);
    cyclonite::core::RingBuffer<uint32_t, 100, true> buffer{ external_buffer.data() };

    EXPECT_TRUE(buffer.empty());
    EXPECT_EQ(buffer.readableSize(), 0);
    EXPECT_EQ(buffer.freeSize(), 100);
}

TEST(ExternalTypedBufferTest, DataAccess)
{
    std::vector<uint32_t> external_buffer(100);
    cyclonite::core::RingBuffer<uint32_t, 100, true> buffer{ external_buffer.data() };

    EXPECT_EQ(buffer.data(), external_buffer.data());

    auto view = buffer.reserveToWrite(1);
    view.data()[0] = uint32_t{ 42 };
    EXPECT_EQ(external_buffer[0], uint32_t{ 42 });
}

TEST(RingBufferEdgeCases, SingleElementBuffer)
{
    cyclonite::core::RingBuffer<int, 1> buffer{};
    auto view1 = buffer.reserveToWrite(1);
    EXPECT_EQ(view1.count(), 1);
    EXPECT_EQ(buffer.freeSize(), 0);

    auto popped = buffer.pop();
    EXPECT_EQ(popped.count(), 1);
    EXPECT_TRUE(buffer.empty());

    auto view2 = buffer.reserveToWrite(1);
    EXPECT_EQ(view2.count(), 1);
}

TEST(ByteRingBufferTest, Alignment)
{
    cyclonite::core::RingBuffer<std::byte, 1024> buffer{};
    auto intView = buffer.reserveToWrite<int>(1);
    EXPECT_EQ(reinterpret_cast<uintptr_t>(intView.data()) % alignof(int), 0);

    buffer.pop();

    auto doubleView = buffer.reserveToWrite<double>(1);
    EXPECT_EQ(reinterpret_cast<uintptr_t>(doubleView.data()) % alignof(double), 0);
}
