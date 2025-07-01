//
// Created by anton on 7/1/25.
//
#include <core/ringBuffer.h>
#include <gtest/gtest.h>
#include <vector>

#ifndef RINGBUFFERTESTS_H
#define RINGBUFFERTESTS_H

// Test fixture for byte RingBuffer with internal storage
class ByteRingBufferTest : public testing::Test
{
protected:
    cyclonite::core::RingBuffer<std::byte, 1024> buffer_;
};

// Test fixture for typed RingBuffer with internal storage
template<typename T>
class TypedRingBufferTest : public testing::Test
{
protected:
    cyclonite::core::RingBuffer<T, 100> buffer_;
};

// Test fixture for byte RingBuffer with external storage
class ExternalByteRingBufferTest : public testing::Test
{
protected:
    ExternalByteRingBufferTest()
      : externalBuffer_(1024)
      , buffer_{ externalBuffer_.data() }
    {
    }

    std::vector<std::byte> externalBuffer_;
    cyclonite::core::RingBuffer<std::byte, 1024, true> buffer_;
};

// Test fixture for typed RingBuffer with external storage
template<typename T>
class ExternalTypedRingBufferTest : public testing::Test
{
protected:
    ExternalTypedRingBufferTest()
      : externalBuffer_(100)
      , buffer_{ externalBuffer_.data() }
    {
    }

    std::vector<T> externalBuffer_;
    cyclonite::core::RingBuffer<T, 100, true> buffer_;
};

// Instantiate typed tests
using TestTypes = testing::Types<int, float, double, char>;
TYPED_TEST_SUITE(TypedRingBufferTest, TestTypes);
TYPED_TEST_SUITE(ExternalTypedRingBufferTest, TestTypes);

#endif // RINGBUFFERTESTS_H
