//
// Created by anton on 6/19/25.
//

#ifndef RINGBUFFER_H
#define RINGBUFFER_H

#include "bufferView.h"
#include <cassert>
#include <concepts>
#include <cstddef>
#include <deque>
#include <limits>
#include <type_traits>
#include <utility>

namespace cyclonite::core {
// for case when we do not need to store any data
// but only manage a range with circular buffer maner
template<size_t Size>
class RingRange
{
public:
    constexpr static size_t invalid_offset_v = std::numeric_limits<size_t>::max();

    RingRange() = default;

    [[nodiscard]] auto readableSize() const -> size_t { return writeOffset_ - readOffset_; }

    [[nodiscard]] auto freeSize() const -> size_t { return Size - readableSize(); }

    [[nodiscard]] auto contiguousFreeSize() const -> size_t;

    [[nodiscard]] auto empty() const -> bool { return readableSize() == 0; }

    [[nodiscard]] auto expectedOffset(size_t size, bool forceShiftToBegin = false) const -> std::pair<size_t, size_t>;

    auto reserveRange(size_t size, bool forceShiftToBegin = false) -> size_t;

    auto popRange() -> std::pair<size_t, size_t>;

private:
    void avoidOverflow();

    [[nodiscard]] auto writeOffset() const -> size_t { return writeOffset_ % Size; }

    [[nodiscard]] auto readOffset() const -> size_t { return readOffset_ % Size; }

    size_t writeOffset_;
    size_t readOffset_;
    std::deque<size_t> reservations_;
};
#include "ringRange.inl"

// allow to bind reserved ranges with some conditions (with a fence, for example)
template<size_t Size, typename ConditionValueType>
class ConditionalRingRange : protected RingRange<Size>
{
public:
    ConditionalRingRange() = default;

    using RingRange<Size>::readableSize;

    using RingRange<Size>::freeSize;

    using RingRange<Size>::contiguousFreeSize;

    using RingRange<Size>::empty;

    using RingRange<Size>::expectedOffset;

    template<typename ConditionType>
        requires std::is_nothrow_convertible_v<ConditionValueType, std::decay_t<ConditionType>>
    auto reserveRange(ConditionType&& condition, size_t size, bool forceShiftToBegin = false) -> size_t;

    template<typename Pred>
        requires(std::invocable<Pred, ConditionValueType const&> &&
                 std::is_same_v<bool, std::invoke_result_t<Pred, ConditionValueType const&>>)
    auto popRange(Pred&& predicate) -> std::pair<size_t, size_t>;

protected:
    std::deque<ConditionValueType> conditions_;
};
#include "conditionalRingRange.inl"

namespace internal {
// mixins
template<typename ElementType,
         size_t Size,
         bool hasExternalBuffer,
         template<typename, size_t, bool>
         class RingBufferType>
class elements_ring_range_t : protected RingRange<Size>
{
    using element_type_t = ElementType;
    using element_type_ptr_t = std::add_pointer_t<ElementType>;
    using return_type_t =
      std::conditional_t<std::is_same_v<element_type_t, std::byte>, size_t, BufferView<element_type_t>>;
    using ring_buffer_t = RingBufferType<element_type_t, Size, hasExternalBuffer>;
    using ring_buffer_ptr_t = std::add_pointer_t<ring_buffer_t>;

public:
    using RingRange<Size>::readableSize;

    using RingRange<Size>::freeSize;

    using RingRange<Size>::contiguousFreeSize;

    using RingRange<Size>::empty;

    auto reserveToWrite(size_t count) -> return_type_t;

    auto pop() -> return_type_t;

protected:
    template<typename T>
        requires(std::is_member_function_pointer_v<decltype(&T::data)> &&
                   []<typename Ret>(Ret (T::*)()) constexpr -> bool {
                    return std::is_same_v<element_type_t, Ret>;
                }(&T::data))
    auto getData(T&& t) -> element_type_ptr_t
    {
        return t.data();
    }

    elements_ring_range_t() = default;
};

template<size_t Size, bool hasExternalBuffer, template<typename, size_t, bool> class RingBufferType>
class bytes_ring_range_t : protected elements_ring_range_t<std::byte, Size, hasExternalBuffer, RingBufferType>
{
    using element_type_t = std::byte;
    using element_type_ptr_t = std::add_pointer_t<std::byte>;
    using ring_buffer_t = RingBufferType<element_type_t, Size, hasExternalBuffer>;
    using ring_buffer_ptr_t = std::add_pointer_t<ring_buffer_t>;

public:
    using RingRange<Size>::readableSize;

    using RingRange<Size>::freeSize;

    using RingRange<Size>::contiguousFreeSize;

    using RingRange<Size>::empty;

    using elements_ring_range_t<std::byte, Size, hasExternalBuffer, RingBufferType>::pop;

    template<typename DataType>
    auto reserveToWrite(size_t count) -> BufferView<DataType>;

protected:
    bytes_ring_range_t() = default;

    template<typename DataType>
    auto reserveAlignedRange([[maybe_unused]] size_t offset,
                             size_t count,
                             size_t alignedByteCount,
                             std::byte* alignedPtr) -> std::add_pointer_t<DataType>;
};

template<typename ElementType,
         typename ConditionValueType,
         size_t Size,
         bool hasExternalBuffer,
         template<typename, typename, size_t, bool>
         class RingBufferType>
class conditional_elements_ring_range_t : protected ConditionalRingRange<Size, ConditionValueType>
{
    using conditional_type_t = ConditionValueType;
    using element_type_t = ElementType;
    using element_type_ptr_t = std::add_pointer_t<ElementType>;
    using return_type_t =
      std::conditional_t<std::is_same_v<ElementType, std::byte>, size_t, BufferView<element_type_t>>;
    using ring_buffer_t = RingBufferType<ElementType, ConditionValueType, Size, hasExternalBuffer>;
    using ring_buffer_ptr_t = std::add_pointer_t<ring_buffer_t>;

public:
    template<typename ConditionType>
        requires std::is_same_v<ConditionValueType, std::decay_t<ConditionType>>
    auto reserveToWrite(ConditionType&& condition, size_t count) -> return_type_t;

    template<typename ConditionType, typename Pred>
        requires(std::is_same_v<ConditionValueType, std::decay_t<ConditionType>> &&
                 std::invocable<Pred, ConditionType &&> &&
                 std::is_same_v<bool, std::invoke_result_t<Pred, ConditionType &&>>)
    auto pop(Pred&& predicate) -> return_type_t;

    auto forcePop() -> return_type_t;

    using RingRange<Size>::readableSize;

    using RingRange<Size>::freeSize;

    using RingRange<Size>::contiguousFreeSize;

    using RingRange<Size>::empty;

protected:
    using ConditionalRingRange<Size, ConditionValueType>::popRange;

    conditional_elements_ring_range_t() = default;
};

template<typename ConditionValueType,
         size_t Size,
         bool hasExternalBuffer,
         template<typename, typename, size_t, bool>
         class RingBufferType>
class conditional_bytes_ring_range_t
  : protected conditional_elements_ring_range_t<std::byte, ConditionValueType, Size, hasExternalBuffer, RingBufferType>
{
    using conditional_type_t = ConditionValueType;
    using element_type_t = std::byte;
    using element_type_ptr_t = std::add_pointer_t<std::byte>;
    using ring_buffer_t = RingBufferType<element_type_t, conditional_type_t, Size, hasExternalBuffer>;
    using ring_buffer_ptr_t = std::add_pointer_t<ring_buffer_t>;

public:
    template<typename ConditionType, typename DataType>
        requires std::is_same_v<ConditionValueType, std::decay_t<ConditionType>>
    auto reserveToWrite(ConditionType&& condition, size_t count) -> BufferView<DataType>;

    template<typename ConditionType>
        requires std::is_same_v<ConditionValueType, std::decay_t<ConditionType>>
    auto reserveToWrite(ConditionType&& condition, size_t align, size_t count) -> BufferView<std::byte>;

    using conditional_elements_ring_range_t<std::byte, ConditionValueType, Size, hasExternalBuffer, RingBufferType>::
      pop;

    using conditional_elements_ring_range_t<std::byte, ConditionValueType, Size, hasExternalBuffer, RingBufferType>::
      forcePop;

    using RingRange<Size>::readableSize;

    using RingRange<Size>::freeSize;

    using RingRange<Size>::contiguousFreeSize;

    using RingRange<Size>::empty;

protected:
    conditional_bytes_ring_range_t() = default;

    template<typename DataType, typename ConditionType>
        requires std::is_same_v<ConditionValueType, std::decay_t<ConditionType>>
    auto reserveAlignedRange([[maybe_unused]] size_t offset,
                             size_t count,
                             size_t alignedByteCount,
                             ConditionType&& condition,
                             std::byte* alignedPtr) -> std::add_pointer_t<DataType>;
};
#include "ringBufferMixins.inl"
}

// circular buffer
// common template
template<typename DataType, size_t Size, bool hasExternalBuffer = false>
class RingBuffer;

// partial specialization #1: RingBuffer around byte array
// allow to present reserved range with a view of necessary type
template<size_t Size>
class RingBuffer<std::byte, Size, false> : public internal::bytes_ring_range_t<Size, false, RingBuffer>
{
    friend class internal::bytes_ring_range_t<Size, false, RingBuffer>;

public:
    RingBuffer() = default;

    [[nodiscard]] auto data() const -> std::byte const* { return buffer_.data(); }
    [[nodiscard]] auto data() -> std::byte* { return buffer_.data(); }

private:
    std::array<std::byte, Size> buffer_;
};

// partial specialization #2: RingBuffer around array of custom (default constructable) type
// allow to iterate over reserved memory
template<typename ElementType, size_t Size>
class RingBuffer<ElementType, Size, false>
  : public internal::elements_ring_range_t<ElementType, Size, false, RingBuffer>
{
    friend class internal::elements_ring_range_t<ElementType, Size, false, RingBuffer>;

    using element_type_t = ElementType;
    using element_type_ptr_t = std::add_pointer_t<element_type_t>;

public:
    RingBuffer() = default;

    [[nodiscard]] auto data() const -> element_type_t const* { return elements_.data(); }
    [[nodiscard]] auto data() -> element_type_ptr_t { return elements_.data(); }

private:
    std::array<element_type_t, Size> elements_;
};

// partial specialization #3: RingBuffer around external byte array
// allow to present reserved range with a view of necessary type
template<size_t Size>
class RingBuffer<std::byte, Size, true> : public internal::bytes_ring_range_t<Size, true, RingBuffer>
{
    friend class internal::bytes_ring_range_t<Size, true, RingBuffer>;

public:
    explicit RingBuffer(std::byte* buffer);

    [[nodiscard]] auto data() const -> std::byte const* { return buffer_; }
    [[nodiscard]] auto data() -> std::byte* { return buffer_; }

private:
    std::byte* buffer_;
};

// partial specialization #4: RingBuffer around external array of custom (default constructable) type
// allow to iterate over reserved memory
template<typename ElementType, size_t Size>
class RingBuffer<ElementType, Size, true> : public internal::elements_ring_range_t<ElementType, Size, true, RingBuffer>
{
    friend class internal::elements_ring_range_t<ElementType, Size, true, RingBuffer>;

    using element_type_t = ElementType;
    using element_type_ptr_t = std::add_pointer_t<element_type_t>;

public:
    explicit RingBuffer(element_type_ptr_t elements);

    [[nodiscard]] auto data() const -> element_type_t const* { return elements_; }
    [[nodiscard]] auto data() -> element_type_ptr_t { return elements_; }

private:
    element_type_ptr_t elements_;
};
#include "ringBuffer.inl" // TODO:: add tests
}

#endif // RINGBUFFER_H
