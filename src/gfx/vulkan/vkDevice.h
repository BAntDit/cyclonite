//
// Created by bantdit on 9/4/19.
//

#ifndef CYCLONITE_DEVICE_H
#define CYCLONITE_DEVICE_H

#include "core/resourceBase.h"
#include "core/resourceRef.h"
#include "gfx/common.h"
#include "gfx/config.h"
#include "handle.h"
#include "vmaUsage.h"
#include <memory>
#include <optional>
#include <string_view>

#if defined(GFX_DRIVER_VULKAN)
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

    ~Device();

    [[nodiscard]] auto allocator() const -> VmaAllocator { return vmaAllocator_; }

    [[nodiscard]] auto vulkanInstance() const -> VkInstance { return vkInstance_; }

    [[nodiscard]] auto physicalDevice() const -> VkPhysicalDevice { return vkPhysicalDevice_; }

    [[nodiscard]] auto graphicsQueueFamilyIndex() const -> uint32_t { return graphicsQueueFamilyIndex_; }

    [[nodiscard]] auto transferQueueFamilyIndex() const -> uint32_t { return transferQueueFamilyIndex_; }

    [[nodiscard]] auto computeQueueFamilyIndex() const -> uint32_t { return computeQueueFamilyIndex_; }

    [[nodiscard]] auto handle() const -> VkDevice { return static_cast<VkDevice>(vkDevice_); }

    [[nodiscard]] auto name() const -> std::string_view { return name_; }

    [[nodiscard]] auto vendor() const -> DeviceVendor { return vendor_; }

    [[nodiscard]] auto limits() const -> DeviceLimits const& { return limits_; }

    [[nodiscard]] auto createRenderWindow(uint32_t width,
                                          uint32_t height,
                                          std::string_view title,
                                          SurfaceFlagBits flags) -> core::ResourceRef;

    [[nodiscard]] auto createTexture(GpuMemoryAllocationFlagBits allocationFlags,
                                     TextureCreationFlagBits imageCreateFlags,
                                     TextureType textureType,
                                     Format format,
                                     uint32_t width,
                                     uint32_t height,
                                     uint32_t depth,
                                     uint32_t mipCount,
                                     uint32_t arrayLayerCount,
                                     TextureTiling tiling,
                                     TextureUsageFlagBits usageFlags) -> core::ResourceRef;

    [[nodiscard]] auto createRenderPassWithRTVs(
      core::ResourceRef depthStencilRef,
      std::array<core::ResourceRef, compile_time_config_t::max_color_attachment_count_v> colorAttachmentRefs,
      std::array<std::pair<uint16_t, uint16_t>, compile_time_config_t::max_color_attachment_count_v>
        colorAttachmentSubresDescs,
      uint32_t width,
      uint32_t height) -> core::ResourceRef;

    [[nodiscard]] auto createRenderPassWithRenderWindow(core::ResourceRef renderWindowRef)
      -> core::ResourceRef;

    using core::ResourceBase::resourceBase;

private:
    VkInstance vkInstance_;
    VkPhysicalDevice vkPhysicalDevice_;
    std::string name_;
    DeviceVendor vendor_;
    DeviceLimits limits_;
    Handle<VkDevice> vkDevice_;
    uint32_t graphicsQueueFamilyIndex_;
    uint32_t transferQueueFamilyIndex_;
    uint32_t computeQueueFamilyIndex_;
    Handle<VkQueue> graphicsQueue_;
    Handle<VkQueue> transferQueue_;
    Handle<VkQueue> computeQueue_;
    VmaAllocator vmaAllocator_;
};
}

#endif // GFX_DRIVER_VULKAN

#endif // CYCLONITE_DEVICE_H
