
template<typename ElementType,
         size_t Size,
         bool hasExternalBuffer,
         template<typename, size_t, bool>
         class RingBufferType>
auto elements_ring_range_t<ElementType, Size, hasExternalBuffer, RingBufferType>::reserveToWrite(size_t count)
  -> return_type_t
{
    auto offset = RingRange<Size>::reserveRange(count);

    if constexpr (std::is_same_v<ElementType, std::byte>) {
        return offset;
    } else {
        auto* rb = static_cast<ring_buffer_ptr_t>(this);
        auto* ptr = (offset != RingRange<Size>::invalid_offset_v) ? rb->data() + offset : element_type_ptr_t{ nullptr };

        return return_type_t{ ptr, offset, count };
    }
}

template<typename ElementType,
         size_t Size,
         bool hasExternalBuffer,
         template<typename, size_t, bool>
         class RingBufferType>
auto elements_ring_range_t<ElementType, Size, hasExternalBuffer, RingBufferType>::pop() -> return_type_t
{
    auto [offset, count] = RingRange<Size>::popRange();

    if constexpr (std::is_same_v<ElementType, std::byte>) {
        return count;
    } else {
        auto* rb = static_cast<ring_buffer_ptr_t>(this);
        auto* ptr = (offset != RingRange<Size>::invalid_offset_v) ? rb->data() + offset : element_type_ptr_t{ nullptr };

        return return_type_t{ ptr, offset, count };
    }
}

template<size_t Size, bool hasExternalBuffer, template<typename, size_t, bool> class RingBufferType>
template<typename DataType>
auto bytes_ring_range_t<Size, hasExternalBuffer, RingBufferType>::reserveToWrite(size_t count) -> BufferView<DataType>
{
    auto* rb = static_cast<ring_buffer_ptr_t>(this);
    auto* ptr = std::add_pointer_t<DataType>{ nullptr };
    auto size = sizeof(DataType) * count;
    auto viewOffset = RingRange<Size>::invalid_offset_v;

    if (auto [offset, available] = RingRange<Size>::expectedOffset(size); offset != RingRange<Size>::invalid_offset_v) {
        auto* basePtr = reinterpret_cast<void*>(rb->data() + offset);
        auto space = available;
        if (auto* alignedPtr = reinterpret_cast<std::byte*>(std::align(alignof(DataType), size, basePtr, space));
            alignedPtr != nullptr) {
            assert(available >= space);
            auto diff = available - space;
            viewOffset = offset + diff;

            ptr = reserveAlignedRange<DataType>(offset, count, diff, alignedPtr);
        } else if (auto [offset2, available2] = RingRange<Size>::expectedOffset(size, true);
                   offset2 != RingRange<Size>::invalid_offset_v) {
            basePtr = reinterpret_cast<void*>(rb->data() + offset2);
            space = available2;
            if (auto* alignedPtr2 = reinterpret_cast<std::byte*>(std::align(alignof(DataType), size, basePtr, space));
                alignedPtr2 != nullptr) {
                assert(available2 >= space);
                auto diff = available2 - space;
                viewOffset = offset2 + diff;

                ptr = reserveAlignedRange<DataType>(offset2, count, diff, alignedPtr2);
            }
        }
    }

    return BufferView<DataType>{ ptr, viewOffset, count };
}

template<size_t Size, bool hasExternalBuffer, template<typename, size_t, bool> class RingBufferType>
template<typename DataType>
auto bytes_ring_range_t<Size, hasExternalBuffer, RingBufferType>::reserveAlignedRange([[maybe_unused]] size_t offset,
                                                                                      size_t count,
                                                                                      size_t alignedByteCount,
                                                                                      std::byte* alignedPtr)
  -> std::add_pointer_t<DataType>
{
    auto* ptr = std::add_pointer_t<DataType>{ nullptr };
    auto size = sizeof(DataType) * count;
    auto alignedSize = size + alignedByteCount;

    if (auto alignedOffset = RingRange<Size>::reserveRange(alignedSize);
        alignedOffset != RingRange<Size>::invalid_offset_v) {
        assert(alignedOffset == offset);
        ptr = reinterpret_cast<DataType*>(alignedPtr);

        if constexpr (!std::is_same_v<DataType, std::byte>) {
            std::uninitialized_value_construct_n(ptr, count);
        }
    }

    return ptr;
}

template<typename ElementType,
         typename ConditionValueType,
         size_t Size,
         bool hasExternalBuffer,
         template<typename, typename, size_t, bool>
         class RingBufferType>
template<typename ConditionType>
    requires std::is_same_v<ConditionValueType, std::decay_t<ConditionType>>
auto conditional_elements_ring_range_t<ElementType, ConditionValueType, Size, hasExternalBuffer, RingBufferType>::
  reserveToWrite(ConditionType&& condition, size_t count) -> return_type_t
{
    auto offset = ConditionalRingRange<Size, ConditionValueType>::reserveRange(condition, count);

    if constexpr (std::is_same_v<ElementType, std::byte>) {
        return offset;
    } else {
        auto* rb = static_cast<ring_buffer_ptr_t>(this);
        auto* ptr = (offset != RingRange<Size>::invalid_offset_v) ? getData(rb) + offset
                                                                  : std::add_pointer_t<element_type_t>{ nullptr };

        return return_type_t{ ptr, offset, count };
    }
}

template<typename ElementType,
         typename ConditionValueType,
         size_t Size,
         bool hasExternalBuffer,
         template<typename, typename, size_t, bool>
         class RingBufferType>
template<typename ConditionType, typename Pred>
    requires(std::is_same_v<ConditionValueType, std::decay_t<ConditionType>> &&
             std::invocable<Pred, ConditionType &&> &&
             std::is_same_v<bool, std::invoke_result_t<Pred, ConditionType &&>>)
auto conditional_elements_ring_range_t<ElementType, ConditionValueType, Size, hasExternalBuffer, RingBufferType>::pop(
  Pred&& predicate) -> return_type_t
{
    auto* ptr = element_type_ptr_t{ nullptr };
    auto offset = RingRange<Size>::invalid_offset_v;
    auto count = size_t{ 0 };

    auto& conditions = ConditionalRingRange<Size, ConditionValueType>::conditions_;
    if (predicate(conditions.back())) {
        conditions.pop_back();

        auto [ofs, cnt] = RingRange<Size>::popRange();
        offset = ofs;
        count = cnt;

        if constexpr (!std::is_same_v<ElementType, std::byte>) {
            auto* rb = static_cast<ring_buffer_ptr_t>(this);
            ptr = (offset != RingRange<Size>::invalid_offset_v) ? getData(*rb) + offset : element_type_ptr_t{ nullptr };
        }
    }

    if constexpr (std::is_same_v<ElementType, std::byte>) {
        return count;
    } else {
        return return_type_t{ ptr, offset, count };
    }
}

template<typename ElementType,
         typename ConditionValueType,
         size_t Size,
         bool hasExternalBuffer,
         template<typename, typename, size_t, bool>
         class RingBufferType>
auto conditional_elements_ring_range_t<ElementType, ConditionValueType, Size, hasExternalBuffer, RingBufferType>::
  forcePop() -> return_type_t
{
    auto [offset, count] = RingRange<Size>::popRange();
    auto* ptr = element_type_ptr_t{ nullptr };

    if constexpr (!std::is_same_v<ElementType, std::byte>) {
        auto* rb = static_cast<ring_buffer_ptr_t>(this);
        ptr = (offset != RingRange<Size>::invalid_offset_v) ? getData(*rb) + offset : element_type_ptr_t{ nullptr };
    }

    if constexpr (std::is_same_v<ElementType, std::byte>) {
        return count;
    } else {
        return return_type_t{ ptr, offset, count };
    }
}

template<typename ConditionValueType,
         size_t Size,
         bool hasExternalBuffer,
         template<typename, typename, size_t, bool>
         class RingBufferType>
template<typename DataType, typename ConditionType>
    requires std::is_same_v<ConditionValueType, std::decay_t<ConditionType>>
auto conditional_bytes_ring_range_t<ConditionValueType, Size, hasExternalBuffer, RingBufferType>::reserveAlignedRange(
  [[maybe_unused]] size_t offset,
  size_t count,
  size_t alignedByteCount,
  ConditionType&& condition,
  std::byte* alignedPtr) -> std::add_pointer_t<DataType>
{
    auto* ptr = std::add_pointer_t<DataType>{ nullptr };
    auto size = sizeof(DataType) * count;
    auto alignedSize = size + alignedByteCount;

    if (auto alignedOffset = ConditionalRingRange<Size, ConditionValueType>::reserveRange(condition, alignedSize);
        alignedOffset != RingRange<Size>::invalid_offset_v) {
        assert(alignedOffset == offset);
        ptr = reinterpret_cast<DataType*>(alignedPtr);

        if constexpr (!std::is_same_v<DataType, std::byte>) {
            std::uninitialized_value_construct_n(ptr, count);
        }
    }

    return ptr;
}

template<typename ConditionValueType,
         size_t Size,
         bool hasExternalBuffer,
         template<typename, typename, size_t, bool>
         class RingBufferType>
template<typename ConditionType, typename DataType>
    requires std::is_same_v<ConditionValueType, std::decay_t<ConditionType>>
auto conditional_bytes_ring_range_t<ConditionValueType, Size, hasExternalBuffer, RingBufferType>::reserveToWrite(
  ConditionType&& condition,
  size_t count) -> BufferView<DataType>
{
    auto* rb = static_cast<ring_buffer_ptr_t>(this);
    auto* ptr = std::add_pointer_t<DataType>{ nullptr };
    auto size = sizeof(DataType) * count;
    auto viewOffset = RingRange<Size>::invalid_offset_v;

    if (auto [offset, available] = RingRange<Size>::expectedRange(size); offset != RingRange<Size>::invalid_offset_v) {
        auto* basePtr = reinterpret_cast<void*>(getData(*rb) + offset);
        auto space = available;
        if (auto* alignedPtr = reinterpret_cast<std::byte*>(std::align(alignof(DataType), size, basePtr, space));
            alignedPtr != nullptr) {
            assert(available >= space);
            auto diff = available - space;
            viewOffset = offset + diff;
            ptr = reserveAlignedRange<DataType>(offset, count, diff, condition, alignedPtr);
        } else if (auto [offset2, available2] = RingRange<Size>::expectedRange(size, true);
                   offset2 != RingRange<Size>::invalid_offset_v) {
            basePtr = reinterpret_cast<void*>(getData(*rb) + offset2);
            space = available2;
            if (auto* alignedPtr = reinterpret_cast<std::byte*>(std::align(alignof(DataType), size, basePtr, space));
                alignedPtr != nullptr) {
                assert(available2 >= space);
                auto diff = available2 - space;
                viewOffset = offset2 + diff;
                ptr = reserveAlignedRange<DataType>(offset2, count, diff, condition, alignedPtr);
            }
        }
    }

    return BufferView{ ptr, viewOffset, count };
}

template<typename ConditionValueType,
         size_t Size,
         bool hasExternalBuffer,
         template<typename, typename, size_t, bool>
         class RingBufferType>
template<typename ConditionType>
    requires std::is_same_v<ConditionValueType, std::decay_t<ConditionType>>
auto conditional_bytes_ring_range_t<ConditionValueType, Size, hasExternalBuffer, RingBufferType>::reserveToWrite(
  ConditionType&& condition,
  size_t align,
  size_t count) -> BufferView<std::byte>
{
    auto* rb = static_cast<ring_buffer_ptr_t>(this);
    auto* ptr = std::add_pointer_t<std::byte>{ nullptr };
    auto size = count;
    auto viewOffset = RingRange<Size>::invalid_offset_v;

    if (auto [offset, available] = RingRange<Size>::expectedRange(size); offset != RingRange<Size>::invalid_offset_v) {
        auto* basePtr = reinterpret_cast<void*>(getData(*rb) + offset);
        auto space = available;
        if (auto* alignedPtr = reinterpret_cast<std::byte*>(std::align(align, size, basePtr, space));
            alignedPtr != nullptr) {
            assert(available >= space);
            auto diff = available - space;
            viewOffset = offset + diff;
            ptr = reserveAlignedRange<std::byte>(offset, count, diff, condition, alignedPtr);
        } else if (auto [offset2, available2] = RingRange<Size>::expectedRange(size, true);
                   offset2 != RingRange<Size>::invalid_offset_v) {
            basePtr = reinterpret_cast<void*>(getData(*rb) + offset2);
            space = available2;
            if (auto* alignedPtr = reinterpret_cast<std::byte*>(std::align(align, size, basePtr, space));
                alignedPtr != nullptr) {
                assert(available2 >= space);
                auto diff = available2 - space;
                viewOffset = offset2 + diff;
                ptr = reserveAlignedRange<std::byte>(offset2, count, diff, condition, alignedPtr);
            }
        }
    }

    return BufferView{ ptr, viewOffset, count };
}
