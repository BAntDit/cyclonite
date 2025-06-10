//
// Created by anton on 4/25/21.
//

#ifndef CYCLONITE_PASSITERATOR_H
#define CYCLONITE_PASSITERATOR_H

#include "passType.h"
#include "vulkan/handle.h"
#include <bitset>
#include <iterator>
#include <type_traits>
#include <array>

namespace cyclonite::compositor {
namespace internal {
class ExpirationBitsWrapper
{
    using is_expired_func_t = bool (*)(void const*, size_t bitIndex);
    using bits_storage_t = std::array<std::byte, sizeof(std::bitset<32>)>;

public:
    template<size_t bitCount>
    explicit ExpirationBitsWrapper(std::bitset<bitCount> const& bits) noexcept
        requires(sizeof(std::bitset<bitCount>) <= sizeof(bits_storage_t))
      : isExpiredFunc_{ [](void const* b, size_t i) -> bool {
          assert(i < bitCount);
          return reinterpret_cast<std::bitset<bitCount> const*>(b)->test(i);
      } }
      , m_{}
      , bits_{ new(m_.data()) std::bitset<bitCount>(bits) }
    {
    }

    [[nodiscard]] auto isExpired(size_t bitIndex) const -> bool { return isExpiredFunc_(bits_, bitIndex); }

private:
    is_expired_func_t isExpiredFunc_;
    bits_storage_t m_;
    void* bits_;
};
}

class PassIterator
{
public:
    using iterator_category = std::input_iterator_tag;
    using difference_type = std::ptrdiff_t;
    using value_type =
      std::tuple<PassType, VkDescriptorPool, VkDescriptorSetLayout, VkPipelineLayout, VkPipeline, VkDescriptorSet*>;
    using pointer = std::add_pointer_t<value_type>;
    using reference = std::add_lvalue_reference_t<value_type>;
    using const_reference = std::add_const_t<reference>;

    template<size_t bitCount>
    PassIterator(uint32_t passCount,
                 uint32_t cursor,
                 PassType* passType,
                 vulkan::Handle<VkDescriptorPool>* baseDescriptorPool,
                 vulkan::Handle<VkDescriptorSetLayout>* baseDescriptorSetLayout,
                 vulkan::Handle<VkPipelineLayout>* basePipelineLayout,
                 vulkan::Handle<VkPipeline>* basePipeline,
                 VkDescriptorSet* baseDescriptorSet,
                 std::bitset<bitCount> const& descriptorExpirationBits) noexcept;

    PassIterator(PassIterator const&) = default;

    ~PassIterator() = default;

    auto operator=(PassIterator const&) -> PassIterator& = default;

    auto operator++() -> PassIterator&;
    auto operator++(int) -> PassIterator;

    auto operator*() const -> std::
      tuple<PassType, VkDescriptorPool, VkDescriptorSetLayout, VkPipelineLayout, VkPipeline, VkDescriptorSet*, bool>;

    auto operator==(PassIterator const& rhs) const -> bool { return cursor_ == rhs.cursor_; }
    auto operator!=(PassIterator const& rhs) const -> bool { return cursor_ != rhs.cursor_; }

private:
    internal::ExpirationBitsWrapper bitsWrapper_;
    size_t count_;
    size_t cursor_;
    PassType* basePassType_;
    vulkan::Handle<VkDescriptorPool>* baseDescriptorPool_;
    vulkan::Handle<VkDescriptorSetLayout>* baseDescriptorSetLayout_;
    vulkan::Handle<VkPipelineLayout>* basePipelineLayout_;
    vulkan::Handle<VkPipeline>* basePipeline_;
    VkDescriptorSet* baseDescriptorSet_;
};

template<size_t bitCount>
PassIterator::PassIterator(uint32_t passCount,
                           uint32_t cursor,
                           PassType* passType,
                           vulkan::Handle<VkDescriptorPool>* baseDescriptorPool,
                           vulkan::Handle<VkDescriptorSetLayout>* baseDescriptorSetLayout,
                           vulkan::Handle<VkPipelineLayout>* basePipelineLayout,
                           vulkan::Handle<VkPipeline>* basePipeline,
                           VkDescriptorSet* baseDescriptorSet,
                           std::bitset<bitCount> const& descriptorExpirationBits) noexcept
  : bitsWrapper_(descriptorExpirationBits)
  , count_{ passCount }
  , cursor_{ cursor }
  , basePassType_{ passType }
  , baseDescriptorPool_{ baseDescriptorPool }
  , baseDescriptorSetLayout_{ baseDescriptorSetLayout }
  , basePipelineLayout_{ basePipelineLayout }
  , basePipeline_{ basePipeline }
  , baseDescriptorSet_{ baseDescriptorSet }
{
}
}

#endif // CYCLONITE_PASSITERATOR_H
