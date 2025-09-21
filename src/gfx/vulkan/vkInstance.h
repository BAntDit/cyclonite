//
// Created by bantdit on 9/1/19.
//

#ifndef GFX_VK_INSTANCE_H
#define GFX_VK_INSTANCE_H

#include "gfx/resourceManager.h"
#include "handle.h"
#include <string_view>

#if defined(GFX_DRIVER_VULKAN)
namespace cyclonite::gfx::vulkan {
class Instance
{
public:
    explicit Instance(std::string_view applicationName);

    Instance(Instance const&) = delete;

    Instance(Instance&&) = default;

    ~Instance() = default;

    auto operator=(Instance const&) -> Instance& = delete;

    auto operator=(Instance&&) -> Instance& = default;

    [[nodiscard]] auto handle() const -> VkInstance { return static_cast<VkInstance>(vkInstance_); }

    [[nodiscard]] auto physicalDeviceCount() const -> uint32_t { return physicalDeviceList_.size(); }

    [[nodiscard]] auto createDevice(uint32_t deviceId = std::numeric_limits<uint32_t>::max())
      -> core::ResourceUniqueRef;

private:
    [[nodiscard]] auto chooseBestPhysicalDevice() const -> uint32_t;

private:
    std::vector<VkPhysicalDevice> physicalDeviceList_;
    Handle<VkInstance> vkInstance_;
    resource_manager_t resourceManager_;
};
}

#endif // GFX_DRIVER_VULKAN
#endif // GFX_VK_INSTANCE_H
