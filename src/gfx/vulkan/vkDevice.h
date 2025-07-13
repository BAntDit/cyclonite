//
// Created by bantdit on 9/4/19.
//

#ifndef CYCLONITE_DEVICE_H
#define CYCLONITE_DEVICE_H

#if defined(GFX_DRIVER_VULKAN)

#include "core/resourceBase.h"
#include "handle.h"
#include <memory>

namespace cyclonite::gfx {
class ResourceManager;
}

namespace cyclonite::gfx::vulkan {
class Device : public core::ResourceBase
{
public:
    struct Capabilities
    {
        explicit Capabilities(VkPhysicalDeviceLimits const& vkPhysicalDeviceLimits);

        size_t minMemoryMapAlignment;
        size_t minStorageBufferOffsetAlignment;
        size_t minUniformBufferOffsetAlignment;
        size_t minTexelBufferOffsetAlignment;
        uint32_t maxPushConstantsSize;
        size_t maxColorAttachments;
    };

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

    [[nodiscard]] auto name() const -> std::string const& { return name_; }

    [[nodiscard]] auto vendor() const -> std::string const& { return vendor_; }

    [[nodiscard]] auto graphicsQueue() const -> VkQueue;

    [[nodiscard]] auto computeQueue() const -> VkQueue;

    [[nodiscard]] auto hostTransferQueue() const -> VkQueue;

    [[nodiscard]] auto graphicsQueueFamilyIndex() const -> uint32_t;

    [[nodiscard]] auto computeQueueFamilyIndex() const -> uint32_t;

    [[nodiscard]] auto hostTransferQueueFamilyIndex() const -> uint32_t;

    // [[nodiscard]] auto capabilities() const -> Capabilities const& { return capabilities_; }

    //[[nodiscard]] auto memoryManager() const -> MemoryManager const& { return *memoryManager_; }

    //[[nodiscard]] auto memoryManager() -> MemoryManager& { return *memoryManager_; }

    [[nodiscard]] auto queueFamilyIndices() const -> std::vector<uint32_t> const& { return queueFamilyIndices_; }

    //[[nodiscard]] auto commandPool() -> CommandPool& { return *commandPool_; }

    //[[nodiscard]] auto commandPoolPtr() -> std::shared_ptr<CommandPool>& { return commandPool_; }

    using core::ResourceBase::resourceBase;

private:
    // Capabilities capabilities_;
    VkInstance vkInstance_;
    VkPhysicalDevice vkPhysicalDevice_;
    std::string name_;
    std::string vendor_;
    size_t graphicsQueueIndex_;
    size_t computeQueueIndex_;
    size_t deviceHostTransferQueueIndex_;
    Handle<VkDevice> vkDevice_;
    std::vector<uint32_t> queueFamilyIndices_;
    std::vector<Handle<VkQueue>> vkQueues_;
    // std::unique_ptr<MemoryManager> memoryManager_;
    // std::shared_ptr<CommandPool> commandPool_;
};
}

#endif // GFX_DRIVER_VULKAN

#endif // CYCLONITE_DEVICE_H
