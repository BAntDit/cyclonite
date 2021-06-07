//
// Created by anton on 4/25/21.
//

#ifndef CYCLONITE_PASSITERATOR_H
#define CYCLONITE_PASSITERATOR_H

#include "passType.h"
#include "vulkan/handle.h"
#include <iterator>
#include <type_traits>

namespace cyclonite::compositor {
class PassIterator
{
public:
    using iterator_category = std::input_iterator_tag;
    using difference_type = std::ptrdiff_t;
    using value_type = std::tuple<PassType,
                                  VkDescriptorPool,
                                  VkDescriptorSetLayout,
                                  VkPipelineLayout,
                                  VkPipeline,
                                  VkDescriptorSet*,
                                  std::byte*>;
    using pointer = std::add_pointer_t<value_type>;
    using reference = std::add_lvalue_reference_t<value_type>;
    using const_reference = std::add_const_t<reference>;

    PassIterator(uint32_t passCount,
                 uint32_t cursor,
                 PassType* passType,
                 vulkan::Handle<VkDescriptorPool>* baseDescriptorPool,
                 vulkan::Handle<VkDescriptorSetLayout>* baseDescriptorSetLayout,
                 vulkan::Handle<VkPipelineLayout>* basePipelineLayout,
                 vulkan::Handle<VkPipeline>* basePipeline,
                 VkDescriptorSet* baseDescriptorSet,
                 std::byte* baseExpirationBitsByte) noexcept;

    PassIterator(PassIterator const&) = default;

    ~PassIterator() = default;

    auto operator=(PassIterator const&) -> PassIterator& = default;

    auto operator++() -> PassIterator&;
    auto operator++(int) -> PassIterator;

    auto operator*() const -> std::tuple<PassType,
                                         VkDescriptorPool,
                                         VkDescriptorSetLayout,
                                         VkPipelineLayout,
                                         VkPipeline,
                                         VkDescriptorSet*,
                                         std::byte*>;

    auto operator==(PassIterator const& rhs) const -> bool { return cursor_ == rhs.cursor_; }
    auto operator!=(PassIterator const& rhs) const -> bool { return cursor_ != rhs.cursor_; }

private:
    size_t count_;
    size_t cursor_;
    PassType* basePassType_;
    vulkan::Handle<VkDescriptorPool>* baseDescriptorPool_;
    vulkan::Handle<VkDescriptorSetLayout>* baseDescriptorSetLayout_;
    vulkan::Handle<VkPipelineLayout>* basePipelineLayout_;
    vulkan::Handle<VkPipeline>* basePipeline_;
    VkDescriptorSet* baseDescriptorSet_;
    std::byte* baseExpirationBitsByte_;
};
}

#endif // CYCLONITE_PASSITERATOR_H
