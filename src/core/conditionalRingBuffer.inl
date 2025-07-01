
template<typename ConditionValueType, size_t Size>
ConditionalRingBuffer<std::byte, ConditionValueType, Size, true>::ConditionalRingBuffer(std::byte* buffer)
  : internal::conditional_bytes_ring_range_t<ConditionValueType, Size, true, ConditionalRingBuffer>{}
, buffer_{ buffer }
{
}

template<typename ElementType, typename ConditionValueType, size_t Size>
ConditionalRingBuffer<ElementType, ConditionValueType, Size, true>::ConditionalRingBuffer(
  element_type_ptr_t elements)
  : internal::conditional_elements_ring_range_t<ElementType,
                                                ConditionValueType,
                                                Size,
                                                true,
                                                ConditionalRingBuffer>{}
, elements_{ elements }
{
}
