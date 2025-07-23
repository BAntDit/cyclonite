//
// Created by bantdit on 9/4/19.
//

#ifndef CYCLONITE_DEVICE_H
#define CYCLONITE_DEVICE_H

#if defined(GFX_DRIVER_VULKAN)

#include "core/resourceBase.h"
#include "gfx/common.h"
#include "handle.h"
#include <memory>
#include <string_view>
#include <optional>

namespace cyclonite::gfx {
class ResourceManager;
}

namespace cyclonite::gfx::vulkan {
class Device : public core::ResourceBase
{
public:
    Device(core::ResourceManagerBase* resourceManager,
           core::ResourceId resourceId,
           VkInstance vkInstance,
           VkPhysicalDevice vkPhysicalDevice,
           VkPhysicalDeviceProperties const& physicalDeviceProperties,
           std::vector<const char*> const& requiredExtensions);

    ~Device() = default;

    [[nodiscard]] auto vulkanInstance() const -> VkInstance { return vkInstance_; }

    [[nodiscard]] auto physicalDevice() const -> VkPhysicalDevice { return vkPhysicalDevice_; }

    [[nodiscard]] auto handle() const -> VkDevice { return static_cast<VkDevice>(vkDevice_); }

    [[nodiscard]] auto name() const -> std::string_view { return name_; }

    [[nodiscard]] auto vendor() const -> DeviceVendor { return vendor_; }

    using core::ResourceBase::resourceBase;

private:
    VkInstance vkInstance_;
    VkPhysicalDevice vkPhysicalDevice_;
    std::string name_;
    DeviceVendor vendor_;
    DeviceLimits limits_;
    Handle<VkDevice> vkDevice_;
    Handle<VkQueue> graphicsQueue_;
    Handle<VkQueue> transferQueue_;
    Handle<VkQueue> computeQueue_;
};
}

#endif // GFX_DRIVER_VULKAN

#endif // CYCLONITE_DEVICE_H
