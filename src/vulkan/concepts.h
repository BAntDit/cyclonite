//
// Created by anton on 1/22/23.
//

#ifndef CYCLONITE_VK_CONCEPTS_H
#define CYCLONITE_VK_CONCEPTS_H

#include <ranges>
#include <vulkan/vulkan.h>

namespace cyclonite::vulkan {
template<typename Container>
concept VkDescriptorSetLayoutContainer = std::ranges::contiguous_range<Container>&&
  std::ranges::sized_range<Container>&& std::is_same_v<std::ranges::range_value_t<Container>, VkDescriptorSetLayout>;
}

#endif // CYCLONITE_VK_CONCEPTS_H
