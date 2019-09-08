//
// Created by bantdit on 9/4/19.
//

#ifndef CYCLONITE_DEVICE_H
#define CYCLONITE_DEVICE_H

#include "handle.h"

namespace cyclonite::vulkan {
class Device
{
public:
    Device(VkPhysicalDevice const& vkPhysicalDevice);

    Device(Device const&) = delete;

    Device(Device&&) = default;

    ~Device() = default;

public:
    auto operator=(Device const&) -> Device& = delete;

    auto operator=(Device &&) -> Device& = default;

public:
    auto handle() const -> Handle<VkDevice> const& { return vkDevice_; }

private:
    std::string name_;
    size_t graphicsQueueIndex_;
    size_t computeQueueIndex_;
    size_t deviceHostTransferIndex_;
    size_t presentationQueueIndex_;
    VkPhysicalDevice vkPhysicalDevice_;
    VkPhysicalDeviceProperties vkDeviceProperties_;
    VkPhysicalDeviceMemoryProperties vkPhysicalDeviceMemoryProperties_;
    Handle<VkDevice> vkDevice_;
};
}

#endif // CYCLONITE_DEVICE_H
