//
// Created by anton on 7/1/25.
//

#include "ringBufferTests.h"

// Tests for internal byte buffer
TEST_F(ByteRingBufferTest, InitialState)
{
    EXPECT_TRUE(buffer_.empty());
    EXPECT_EQ(buffer_.readableSize(), 0);
    EXPECT_EQ(buffer_.freeSize(), 1024);
}

TEST_F(ByteRingBufferTest, ReserveAndPop)
{
    auto view = buffer_.reserveToWrite<int>(2);
    ASSERT_NE(view.data(), nullptr);
    EXPECT_EQ(view.count(), 2);
    EXPECT_EQ(buffer_.readableSize(), sizeof(int) * 2);
    EXPECT_EQ(buffer_.freeSize(), 1024 - sizeof(int) * 2);

    auto popped = buffer_.pop();
    EXPECT_EQ(popped, sizeof(int) * 2);
    EXPECT_TRUE(buffer_.empty());
}

TEST_F(ByteRingBufferTest, WrapAround)
{
    // Fill the buffer almost completely
    auto view1 = buffer_.reserveToWrite<std::byte>(500);
    auto view2 = buffer_.reserveToWrite<std::byte>(500);
    EXPECT_EQ(view1.count(), 500);
    EXPECT_EQ(view2.count(), 500);

    // Pop first 500
    buffer_.pop();

    // Now write should wrap around
    auto view3 = buffer_.reserveToWrite<std::byte>(500);
    EXPECT_EQ(view3.count(), 500);
    EXPECT_EQ(view3.data(), view1.data());
}

// Tests for internal typed buffer
TYPED_TEST(TypedRingBufferTest, InitialState)
{
    EXPECT_TRUE(this->buffer_.empty());
    EXPECT_EQ(this->buffer_.readableSize(), 0);
    EXPECT_EQ(this->buffer_.freeSize(), 100);
}

TYPED_TEST(TypedRingBufferTest, ReserveAndPop)
{
    auto view = this->buffer_.reserveToWrite(10);
    ASSERT_NE(view.data(), nullptr);
    EXPECT_EQ(view.count(), 10);
    EXPECT_EQ(this->buffer_.readableSize(), 10);
    EXPECT_EQ(this->buffer_.freeSize(), 90);

    auto popped = this->buffer_.pop();
    EXPECT_EQ(popped.count(), 10);
    EXPECT_TRUE(this->buffer_.empty());
}

TYPED_TEST(TypedRingBufferTest, ContiguousOperations)
{
    // First write
    auto view1 = this->buffer_.reserveToWrite(60);
    EXPECT_EQ(view1.count(), 60);

    // Second write
    auto view2 = this->buffer_.reserveToWrite(30);
    EXPECT_EQ(view2.count(), 30);

    // Pop first write
    this->buffer_.pop();

    // Third write should wrap around
    auto view3 = this->buffer_.reserveToWrite(50);
    EXPECT_EQ(view3.count(), 50);
    EXPECT_EQ(view3.offset(), view1.offset());
}

// Tests for external byte buffer
TEST_F(ExternalByteRingBufferTest, InitialState)
{
    EXPECT_TRUE(buffer_.empty());
    EXPECT_EQ(buffer_.readableSize(), 0);
    EXPECT_EQ(buffer_.freeSize(), 1024);
}

TEST_F(ExternalByteRingBufferTest, DataAccess)
{
    EXPECT_EQ(buffer_.data(), externalBuffer_.data());

    auto view = buffer_.reserveToWrite<int>(1);
    view.data()[0] = 42;
    EXPECT_EQ(externalBuffer_.data()[0], 42);
}

// Tests for external typed buffer
TYPED_TEST(ExternalTypedRingBufferTest, InitialState)
{
    EXPECT_TRUE(this->buffer_.empty());
    EXPECT_EQ(this->buffer_.readableSize(), 0);
    EXPECT_EQ(this->buffer_.freeSize(), 100);
}

TYPED_TEST(ExternalTypedRingBufferTest, DataAccess)
{
    EXPECT_EQ(this->buffer_.data(), this->externalBuffer_.data());

    auto view = this->buffer_.reserveToWrite(1);
    view.data()[0] = TypeParam{ 42 };
    EXPECT_EQ(this->externalBuffer_[0], TypeParam{ 42 });
}

TYPED_TEST(ExternalTypedRingBufferTest, FullCycle)
{
    // Fill the buffer
    for (int i = 0; i < 5; i++) {
        auto view = this->buffer_.reserveToWrite(20);
        for (int j = 0; j < 20; j++) {
            view.data()[j] = TypeParam{ i * 20 + j };
        }
        this->buffer_.pop();
    }

    // Verify the external buffer contains the last write
    for (int j = 0; j < 20; j++) {
        EXPECT_EQ(this->externalBuffer_[j], TypeParam{ 80 + j });
    }
}

// Edge case tests
TEST(RingBufferEdgeCases, SingleElementBuffer)
{
    cyclonite::core::RingBuffer<int, 1> buffer;
    auto view1 = buffer.reserveToWrite(1);
    EXPECT_EQ(view1.count(), 1);
    EXPECT_EQ(buffer.freeSize(), 0);

    auto popped = buffer.pop();
    EXPECT_EQ(popped.count(), 1);
    EXPECT_TRUE(buffer.empty());

    auto view2 = buffer.reserveToWrite(1);
    EXPECT_EQ(view2.count(), 1);
}

// Test alignment requirements
TEST_F(ByteRingBufferTest, Alignment)
{
    auto intView = buffer_.reserveToWrite<int>(1);
    EXPECT_EQ(reinterpret_cast<uintptr_t>(intView.data()) % alignof(int), 0);

    buffer_.pop();

    auto doubleView = buffer_.reserveToWrite<double>(1);
    EXPECT_EQ(reinterpret_cast<uintptr_t>(doubleView.data()) % alignof(double), 0);
}
