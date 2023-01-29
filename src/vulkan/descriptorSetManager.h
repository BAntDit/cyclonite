//
// Created by bantdit on 1/15/23.
//

#ifndef CYCLONITE_DESCRIPTORSETMANAGER_H
#define CYCLONITE_DESCRIPTORSETMANAGER_H

#include "concepts.h"
#include "multithreading/taskManager.h"
#include "pipelineDescriptorSets.h"
#include "sharedHandle.h"
#include <boost/functional/hash.hpp>
#include <unordered_map>

namespace cyclonite::vulkan {
class Device;

class DescriptorSetManager
{
    friend class PipelineDescriptorSets;

public:
    DescriptorSetManager(Device& device, multithreading::TaskManager& taskManager);

    DescriptorSetManager(DescriptorSetManager const&) = delete;

    DescriptorSetManager(DescriptorSetManager&&) = default;

    ~DescriptorSetManager() = default;

    auto operator=(DescriptorSetManager const&) -> DescriptorSetManager& = delete;

    auto operator=(DescriptorSetManager &&) -> DescriptorSetManager& = default;

    auto addPool(DescriptorPoolKey poolKey, uint32_t capacity)
      -> std::tuple<vulkan::SharedHandle<VkDescriptorPool>, size_t&>;

    template<VkDescriptorSetLayoutContainer Container>
    auto allocDescriptorSet(DescriptorPoolKey poolKey, Container&& layouts, uint32_t newPoolCapacity = 32)
      -> PipelineDescriptorSets;

private:
    void freeDescriptorSet(PipelineDescriptorSets& descriptorSets);

    auto _addPool(DescriptorPoolKey poolKey,
                  uint32_t capacity,
                  std::vector<std::tuple<vulkan::SharedHandle<VkDescriptorPool>, size_t, size_t>>& pools)
      -> std::tuple<vulkan::SharedHandle<VkDescriptorPool> const&, size_t&>;

    auto _allocDescriptorSet(DescriptorPoolKey poolKey,
                             VkDescriptorSetLayout const* layouts,
                             uint32_t count,
                             uint32_t newPoolCapacity) -> PipelineDescriptorSets;

    using pool_map_t =
      std::unordered_map<packed_descriptor_set_key_t,
                         std::vector<std::tuple<vulkan::SharedHandle<VkDescriptorPool>, size_t, size_t>>,
                         boost::hash<packed_descriptor_set_key_t>>;

    Device* device_;
    multithreading::TaskManager* taskManager_;
    pool_map_t pools_;
};

template<VkDescriptorSetLayoutContainer Container>
auto DescriptorSetManager::allocDescriptorSet(DescriptorPoolKey poolKey, Container&& layouts, uint32_t newPoolCapacity)
  -> PipelineDescriptorSets
{
    return _allocDescriptorSet(poolKey, std::data(layouts), std::size(layouts), newPoolCapacity);
}
}

#endif // CYCLONITE_DESCRIPTORSETMANAGER_H
