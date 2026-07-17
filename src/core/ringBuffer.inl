
template<size_t Size>
RingBuffer<std::byte, Size, true>::RingBuffer(std::byte* buffer)
  : internal::bytes_ring_range_t<Size, true, RingBuffer>{}
  , buffer_{ buffer }
{
}

template<typename ElementType, size_t Size>
RingBuffer<ElementType, Size, true>::RingBuffer(element_type_ptr_t elements)
  : internal::elements_ring_range_t<ElementType, Size, true, RingBuffer>{}
  , elements_{ elements }
{
}
