//
// Created by bantdit on 1/15/23.
//

#ifndef CYCLONITE_DESCRIPTORSETMANAGER_H
#define CYCLONITE_DESCRIPTORSETMANAGER_H

#include "descriptorSet.h"
#include "sharedHandle.h"
#include <boost/functional/hash.hpp>
#include <unordered_map>

namespace cyclonite::multithreading
{
class TaskManager;
}

namespace cyclonite::vulkan {
class Device;

class DescriptorSetManager
{
public:
    DescriptorSetManager(Device& device, multithreading::TaskManager& taskManager);

    void addPool(DescriptorPoolKey poolKey, uint32_t capacity);

    auto allocDescriptorSet(DescriptorPoolKey poolKey, uint32_t newPoolCapacity = 32) const -> DescriptorSet;

    void freeDescriptorSet(SharedHandle<VkDescriptorPool> const& pool, VkDescriptorSet set);

private:
    using pool_map_t = std::unordered_map<packed_descriptor_set_key_t,
                                          std::vector<vulkan::SharedHandle<VkDescriptorPool>>,
                                          boost::hash<packed_descriptor_set_key_t>>;

    Device* device_;
    multithreading::TaskManager* taskManager_;
    pool_map_t pools_;
};
}

#endif // CYCLONITE_DESCRIPTORSETMANAGER_H
