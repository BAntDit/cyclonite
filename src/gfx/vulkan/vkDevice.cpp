//
// Created by anton on 6/26/25.
//

#include "vkDevice.h"
#include "core/hashTable.h"
#include "gfx/resourceManager.h"
#include "gfx/shader.h"
#include "internal/internalResourceManager.h"
#include "internal/utils.h"
#include "multithreading/executor.h"
#include "multithreading/taskManager.h"
#include "vkException.h"
#include "vkPipelineManager.h"
#include <array>
#include <cstring>
#include <limits>
#include <metrix/enum.h>
#include <ranges>
#include <stdexcept>
#include <tuple>
#include <utility>

#if defined(GFX_DRIVER_VULKAN)

namespace cyclonite::gfx::vulkan {
namespace {
auto testRequiredDeviceExtensions(VkPhysicalDevice physicalDevice, std::vector<const char*> const& requiredExtensions)
{
    auto extensionCount = uint32_t{ 0 };
    auto availableExtensionList = std::vector<VkExtensionProperties>{};

    if (vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &extensionCount, nullptr) != VK_SUCCESS) {
        return false;
    }

    if (extensionCount > 0) {
        availableExtensionList.resize(extensionCount);

        if (vkEnumerateDeviceExtensionProperties(
              physicalDevice, nullptr, &extensionCount, availableExtensionList.data()) != VK_SUCCESS) {
            return false;
        }
    }

    for (auto* reqExtName : requiredExtensions) {
        auto it = availableExtensionList.cbegin();
        while (it != availableExtensionList.cend()) {
            if (0 == strcmp(reqExtName, (*it).extensionName))
                break;
            it++;
        }

        if (it == availableExtensionList.cend()) {
            return false;
        }
    }

    return true;
}

auto getVendorById(uint32_t id) -> DeviceVendor
{
    auto result = DeviceVendor::Unknown;

    switch (id) {
        case metrix::value_cast(DeviceVendor::AMD):
            result = DeviceVendor::AMD;
            break;
        case metrix::value_cast(DeviceVendor::NVIDIA):
            result = DeviceVendor::NVIDIA;
            break;
        case metrix::value_cast(DeviceVendor::ImgTec_PowerVR):
            result = DeviceVendor::ImgTec_PowerVR;
            break;
        case metrix::value_cast(DeviceVendor::ARM_MaliGPU):
            result = DeviceVendor::ARM_MaliGPU;
            break;
        case metrix::value_cast(DeviceVendor::Intel):
            result = DeviceVendor::Intel;
            break;
        case metrix::value_cast(DeviceVendor::Google_SwiftShader_virtualGPU):
            result = DeviceVendor::Google_SwiftShader_virtualGPU;
            break;
        case metrix::value_cast(DeviceVendor::Apple):
            result = DeviceVendor::Apple;
            break;
        case metrix::value_cast(DeviceVendor::QEMU_emulatedGPU):
            result = DeviceVendor::QEMU_emulatedGPU;
            break;
        case metrix::value_cast(DeviceVendor::Qualcomm_AdrenoGPU):
            result = DeviceVendor::Qualcomm_AdrenoGPU;
            break;
        case metrix::value_cast(DeviceVendor::VIATechnologies):
            result = DeviceVendor::VIATechnologies;
            break;
        case metrix::value_cast(DeviceVendor::Vivante):
            result = DeviceVendor::Vivante;
            break;
        case metrix::value_cast(DeviceVendor::VMware_virtualGPU):
            result = DeviceVendor::VMware_virtualGPU;
            break;
        default:
            result = DeviceVendor::Unknown;
    }

    return result;
}

using queue_family_flag_req_t = core::StaticHashTable<uint32_t, 32, uint32_t>;

enum class QueueSelectionPolicy : uint32_t
{
    Required,
    BetterInclude,
    BetterExclude
};

template<size_t N>
auto getBestQueueFamilyIndex(std::vector<VkQueueFamilyProperties> const& familyProperties,
                             queue_family_flag_req_t& usage,
                             std::array<std::pair<VkQueueFlagBits, QueueSelectionPolicy>, N>&& requirements)
  -> std::tuple<uint32_t, uint32_t, VkQueueFlags>
{
    auto flags = VkQueueFlags{};
    auto queueFamilyIndex = std::numeric_limits<uint32_t>::max();
    auto queueIndex = uint32_t{ 0 };

    auto bestCapabilityCount = uint32_t{ 0 };

    for (auto i = uint32_t{ 0 }, count = static_cast<uint32_t>(familyProperties.size()); i < count; i++) {
        auto&& properties = familyProperties[i];

        auto it = usage.find(i);
        if (it != usage.end()) {
            auto [_, v] = *it;
            if (v >= properties.queueCount) {
                continue;
            }
        }

        flags = VkQueueFlags{};
        auto lastCapabilityCount = uint32_t{ 0 };
        auto skipFamily = false;
        for (auto&& [flag, policy] : requirements) {
            if (policy == QueueSelectionPolicy::Required && ((properties.queueFlags & flag) == 0)) {
                skipFamily = true;
                break;
            } else {
                flags = flags | flag;
                lastCapabilityCount++;
            }

            if (policy == QueueSelectionPolicy::BetterInclude && ((properties.queueFlags & flag) != 0)) {
                lastCapabilityCount++;
                flags = flags | flag;
            }

            if (policy == QueueSelectionPolicy::BetterExclude && ((properties.queueFlags & flag) == 0)) {
                lastCapabilityCount++;
            }
        }

        if (skipFamily)
            continue;

        if (lastCapabilityCount > bestCapabilityCount) {
            bestCapabilityCount = lastCapabilityCount;
            queueFamilyIndex = i;
        }
    }

    if (queueFamilyIndex != std::numeric_limits<uint32_t>::max()) {
        auto it = usage.find(queueFamilyIndex);
        if (it == usage.end()) {
            usage.add(queueIndex + 1, queueFamilyIndex);
        } else {
            auto& [_, v] = *it;
            assert(v < familyProperties[queueFamilyIndex].queueCount);
            queueIndex = v++;
        }
    }

    return std::tuple{ queueFamilyIndex, queueIndex, flags };
}
}

Device::Device(core::ResourceManagerBase* resourceManager,
               core::ResourceId resourceId,
               VkInstance vkInstance,
               VkPhysicalDevice vkPhysicalDevice,
               VkPhysicalDeviceProperties const& physicalDeviceProperties,
               std::vector<const char*> const& requiredExtensions)
  : core::ResourceBase{ resourceManager, resourceId, false }
  , core::EnableRefFromThis{}
  , vkInstance_{ vkInstance }
  , vkPhysicalDevice_{ vkPhysicalDevice }
  , name_{ physicalDeviceProperties.deviceName }
  , vendor_{ getVendorById(physicalDeviceProperties.deviceID) }
  , limits_{}
  , vkDevice_{ vkDestroyDevice }
  , graphicsQueueFamilyIndex_{ std::numeric_limits<uint32_t>::max() }
  , transferQueueFamilyIndex_{ std::numeric_limits<uint32_t>::max() }
  , computeQueueFamilyIndex_{ std::numeric_limits<uint32_t>::max() }
  , graphicsQueue_{}
  , transferQueue_{}
  , computeQueue_{}
  , vmaAllocator_{ VK_NULL_HANDLE }
  , internalResourceManager_{ std::make_unique<internal::internal_resource_manager_t>() }
  , pipelineManager_{}
{
    if (!testRequiredDeviceExtensions(vkPhysicalDevice, requiredExtensions)) {
        throw std::runtime_error("gfx:: physical device does not supports required extensions. Device name: " + name_);
    }

    uint32_t familyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(vkPhysicalDevice_, &familyCount, nullptr);

    auto familyPropertiesList = std::vector<VkQueueFamilyProperties>(familyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(vkPhysicalDevice_, &familyCount, familyPropertiesList.data());

    auto familyQueueUsages = queue_family_flag_req_t{};

    auto [graphicsQueueFamilyIndex, graphicsQueueIndex, graphicsQueueFlags] =
      getBestQueueFamilyIndex(familyPropertiesList,
                              familyQueueUsages,
                              std::array{ std::pair{ VK_QUEUE_GRAPHICS_BIT, QueueSelectionPolicy::Required },
                                          std::pair{ VK_QUEUE_TRANSFER_BIT, QueueSelectionPolicy::Required },
                                          std::pair{ VK_QUEUE_COMPUTE_BIT, QueueSelectionPolicy::BetterInclude } });

    auto [transferQueueFamilyIndex, transferQueueIndex, transferQueueFlags] =
      getBestQueueFamilyIndex(familyPropertiesList,
                              familyQueueUsages,
                              std::array{ std::pair{ VK_QUEUE_GRAPHICS_BIT, QueueSelectionPolicy::BetterExclude },
                                          std::pair{ VK_QUEUE_TRANSFER_BIT, QueueSelectionPolicy::Required },
                                          std::pair{ VK_QUEUE_COMPUTE_BIT, QueueSelectionPolicy::BetterExclude } });

    auto [computeQueueFamilyIndex, computeQueueIndex, computeQueueFlags] =
      getBestQueueFamilyIndex(familyPropertiesList,
                              familyQueueUsages,
                              std::array{ std::pair{ VK_QUEUE_GRAPHICS_BIT, QueueSelectionPolicy::BetterExclude },
                                          std::pair{ VK_QUEUE_TRANSFER_BIT, QueueSelectionPolicy::BetterInclude },
                                          std::pair{ VK_QUEUE_COMPUTE_BIT, QueueSelectionPolicy::Required } });

    if (graphicsQueueFamilyIndex == std::numeric_limits<uint32_t>::max()) {
        throw std::runtime_error("gfx:: could not find valid graphics queue family index for device: " + name_);
    }

    assert(transferQueueIndex < 3);

    auto queueCreateInfoCount = uint32_t{ 0 };
    auto deviceQueueCreateInfoArray = std::array<VkDeviceQueueCreateInfo, 3>{};

    auto graphicsQueuePriorities = std::array<float, 3>{ 1.0f, 1.0f, 1.0f };
    if (graphicsQueueFamilyIndex == transferQueueFamilyIndex) {
        graphicsQueuePriorities[transferQueueIndex] = 0.5f;
    }

    auto computeQueuePriorities = std::array<float, 3>{ 1.0f, 1.0f, 1.0f };
    if (computeQueueFamilyIndex == transferQueueFamilyIndex) {
        computeQueuePriorities[transferQueueIndex] = 0.5f;
    }

    auto transferQueuePriorities = std::array<float, 3>{ 1.0f, 1.0f, 1.0f };
    transferQueuePriorities[transferQueueIndex] = 0.5f;

    // graphics
    {
        auto it = familyQueueUsages.find(graphicsQueueFamilyIndex);
        assert(it != familyQueueUsages.end());

        auto const [_, usageCount] = *it;
        assert(usageCount > 0);
        assert(usageCount <= 3);

        familyQueueUsages.remove(it);

        auto& deviceQueueCreateInfo = deviceQueueCreateInfoArray[queueCreateInfoCount++];
        deviceQueueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        deviceQueueCreateInfo.queueFamilyIndex = graphicsQueueFamilyIndex;
        deviceQueueCreateInfo.queueCount = usageCount;
        deviceQueueCreateInfo.pQueuePriorities = graphicsQueuePriorities.data();
    }

    // transfer
    if (transferQueueFamilyIndex != std::numeric_limits<uint32_t>::max()) {
        limits_.dedicatedTransferQueue = true;
        if (auto it = familyQueueUsages.find(transferQueueFamilyIndex); it != familyQueueUsages.end()) {
            auto const [_, usageCount] = *it;
            assert(usageCount > 0);
            assert(usageCount <= 3);

            familyQueueUsages.remove(it);

            auto& deviceQueueCreateInfo = deviceQueueCreateInfoArray[queueCreateInfoCount++];
            deviceQueueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
            deviceQueueCreateInfo.queueFamilyIndex = transferQueueFamilyIndex;
            deviceQueueCreateInfo.queueCount = usageCount;
            deviceQueueCreateInfo.pQueuePriorities = transferQueuePriorities.data();
        }
    }

    // compute
    if (computeQueueFamilyIndex != std::numeric_limits<uint32_t>::max()) {
        limits_.supportCompute = true;
        limits_.dedicatedComputeQueue = true;

        if (auto it = familyQueueUsages.find(computeQueueFamilyIndex); it != familyQueueUsages.end()) {
            auto const [_, usageCount] = *it;
            assert(usageCount > 0);
            assert(usageCount <= 3);

            familyQueueUsages.remove(it);

            auto& deviceQueueCreateInfo = deviceQueueCreateInfoArray[queueCreateInfoCount++];
            deviceQueueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
            deviceQueueCreateInfo.queueFamilyIndex = computeQueueFamilyIndex;
            deviceQueueCreateInfo.queueCount = usageCount;
            deviceQueueCreateInfo.pQueuePriorities = computeQueuePriorities.data();
        }
    } else {
        limits_.supportCompute = ((graphicsQueueFlags & VK_QUEUE_COMPUTE_BIT) != 0);
    }

    auto features = VkPhysicalDeviceFeatures{};

    // turn off unused features (for now)
    features.robustBufferAccess = VK_FALSE;
    features.shaderFloat64 = VK_FALSE;
    features.shaderInt64 = VK_FALSE;
    features.inheritedQueries = VK_FALSE;

    // ext features
    auto indexingFeatures = VkPhysicalDeviceDescriptorIndexingFeatures{};
    indexingFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_INDEXING_FEATURES;

    auto timelineSemaphoreFeatures = VkPhysicalDeviceTimelineSemaphoreFeatures{};
    timelineSemaphoreFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_TIMELINE_SEMAPHORE_FEATURES;
    timelineSemaphoreFeatures.pNext = &indexingFeatures;

    auto features2 = VkPhysicalDeviceFeatures2{};
    features2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
    features2.pNext = &timelineSemaphoreFeatures;

    vkGetPhysicalDeviceFeatures2(vkPhysicalDevice_, &features2);
    if (!timelineSemaphoreFeatures.timelineSemaphore) {
        throw std::runtime_error("select device does not support necessary feature: timeline semaphores");
    }
    if (!indexingFeatures.shaderSampledImageArrayNonUniformIndexing) {
        throw std::runtime_error("selected device does not support sampled image non-uniform indexing");
    }
    if (!indexingFeatures.descriptorBindingSampledImageUpdateAfterBind) {
        throw std::runtime_error("selected device does not support sampled image descriptors update after bind");
    }
    if (!indexingFeatures.shaderUniformBufferArrayNonUniformIndexing) {
        throw std::runtime_error("selected device does not support UBO array non-uniform indexing");
    }
    if (!indexingFeatures.descriptorBindingUniformBufferUpdateAfterBind) {
        throw std::runtime_error("selected device does not support UBO descriptors update after bind");
    }
    if (!indexingFeatures.shaderStorageBufferArrayNonUniformIndexing) {
        throw std::runtime_error("selected device does not support SSBO array non-uniform indexing");
    }
    if (!indexingFeatures.descriptorBindingStorageBufferUpdateAfterBind) {
        throw std::runtime_error("seleced device does not support SSBO descriptors update after bind");
    }

    auto deviceInfo = VkDeviceCreateInfo{};
    deviceInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    deviceInfo.pNext = &features2;
    deviceInfo.queueCreateInfoCount = queueCreateInfoCount;
    deviceInfo.pQueueCreateInfos = deviceQueueCreateInfoArray.data();
    deviceInfo.enabledExtensionCount = static_cast<uint32_t>(requiredExtensions.size());
    deviceInfo.ppEnabledExtensionNames = requiredExtensions.data();
    deviceInfo.pEnabledFeatures = &features;

    if (auto vkResult = vkCreateDevice(vkPhysicalDevice_, &deviceInfo, nullptr, &vkDevice_); vkResult != VK_SUCCESS) {
        throw Exception{ vkResult, "vkCreateDevice" };
    }

    assert(graphicsQueueFamilyIndex != std::numeric_limits<uint32_t>::max());
    vkGetDeviceQueue(static_cast<VkDevice>(vkDevice_), graphicsQueueFamilyIndex, graphicsQueueIndex, &graphicsQueue_);
    graphicsQueueFamilyIndex_ = graphicsQueueFamilyIndex;

    if (transferQueueFamilyIndex != std::numeric_limits<uint32_t>::max()) {
        vkGetDeviceQueue(
          static_cast<VkDevice>(vkDevice_), transferQueueFamilyIndex, transferQueueIndex, &transferQueue_);
        transferQueueFamilyIndex_ = transferQueueFamilyIndex;
    }

    if (computeQueueFamilyIndex != std::numeric_limits<uint32_t>::max()) {
        vkGetDeviceQueue(static_cast<VkDevice>(vkDevice_), computeQueueFamilyIndex, computeQueueIndex, &computeQueue_);
        computeQueueFamilyIndex_ = computeQueueFamilyIndex;
    }

    auto allocatorCreateInfo = VmaAllocatorCreateInfo{};
    allocatorCreateInfo.flags = VMA_ALLOCATOR_CREATE_EXTERNALLY_SYNCHRONIZED_BIT;
    allocatorCreateInfo.physicalDevice = vkPhysicalDevice_;
    allocatorCreateInfo.device = static_cast<VkDevice>(vkDevice_);
    allocatorCreateInfo.instance = vkInstance;
    allocatorCreateInfo.vulkanApiVersion = VK_API_VERSION_1_0;

    if (auto vkResult = vmaCreateAllocator(&allocatorCreateInfo, &vmaAllocator_); vkResult != VK_SUCCESS) {
        throw Exception{ vkResult, "vmaCreateAllocator" };
    }

    limits_.maxColorAttachmentCount = static_cast<uint8_t>(physicalDeviceProperties.limits.maxColorAttachments);

    pipelineManager_ = std::make_unique<PipelineManager>(this);
}

auto Device::createSignal(SignalType signalType, uint64_t initialValue /* = 0*/) -> core::ResourceUniqueRef
{
    auto result = core::ResourceUniqueRef{};

    auto& resManager = static_cast<resource_manager_t&>(resourceManager());

    auto deviceRef = getSharedFromThis(this);

    result = resManager.allocResource<gfx::Signal>(deviceRef, signalType, initialValue);

    return result;
}

auto Device::createBuffer(GpuMemoryAllocationFlagBits allocationFlags,
                          BufferUsageFlagBits usageFlags,
                          size_t size) -> core::ResourceUniqueRef
{
    auto result = core::ResourceUniqueRef{};

    auto& resManager = static_cast<resource_manager_t&>(resourceManager());

    auto deviceRef = getSharedFromThis(this);

    result = resManager.allocResource<gfx::Buffer>(deviceRef, size, allocationFlags, usageFlags);

    return result;
}

auto Device::createTexture(GpuMemoryAllocationFlagBits allocationFlags,
                           TextureCreationFlagBits imageCreateFlags,
                           TextureType textureType,
                           Format format,
                           uint32_t width,
                           uint32_t height,
                           uint32_t depth,
                           uint32_t mipCount,
                           uint32_t arrayLayerCount,
                           TextureTiling tiling,
                           TextureUsageFlagBits usageFlags) -> core::ResourceUniqueRef
{
    auto result = core::ResourceUniqueRef{};

    auto& resManager = static_cast<resource_manager_t&>(resourceManager());

    auto deviceRef = getSharedFromThis(this);

    result = resManager.allocResource<gfx::Texture>(deviceRef,
                                                    allocationFlags,
                                                    imageCreateFlags,
                                                    textureType,
                                                    format,
                                                    width,
                                                    height,
                                                    depth,
                                                    mipCount,
                                                    arrayLayerCount,
                                                    tiling,
                                                    usageFlags);

    return result;
}

auto Device::createShader(size_t codeSize,
                          uint32_t const* code,
                          ShaderStageCreationFlagBits creationFlags,
                          ShaderStageFlags stage,
                          std::string_view entryPointName) -> core::ResourceUniqueRef
{
    auto result = core::ResourceUniqueRef{};

    auto& resManager = static_cast<resource_manager_t&>(resourceManager());

    auto deviceRef = getSharedFromThis(this);

    result = resManager.allocResource<gfx::Shader>(deviceRef, codeSize, code, creationFlags, stage, entryPointName);

    return result;
}

auto Device::createRenderWindow(uint32_t width,
                                uint32_t height,
                                std::string_view title,
                                SurfaceFlagBits flags) -> core::ResourceUniqueRef
{
    auto result = core::ResourceUniqueRef{};

    auto& resManager = static_cast<resource_manager_t&>(resourceManager());

    auto deviceRef = getSharedFromThis(this);

    result = resManager.allocResource<gfx::RenderWindow>(deviceRef, width, height, title, flags);

    return result;
}

auto Device::createRenderPassWithRTVs(
  core::ResourceSharedRef depthStencilRef,
  std::array<core::ResourceSharedRef, config_t::max_color_attachment_count_v> colorAttachmentRefs,
  std::array<std::pair<uint16_t, uint16_t>, config_t::max_color_attachment_count_v> colorAttachmentSubresDescs,
  std::array<gfx::Color, config_t::max_color_attachment_count_v> clearColors,
  uint32_t width,
  uint32_t height,
  real depthClearValue,
  uint8_t stencilClearValue) -> core::ResourceUniqueRef
{
    auto result = core::ResourceUniqueRef{};

    auto& resManager = static_cast<resource_manager_t&>(resourceManager());

    auto deviceRef = getSharedFromThis(this);

    result = resManager.allocResource<gfx::RenderPass>(deviceRef,
                                                       depthStencilRef,
                                                       colorAttachmentRefs,
                                                       colorAttachmentSubresDescs,
                                                       clearColors,
                                                       width,
                                                       height,
                                                       depthClearValue,
                                                       stencilClearValue);

    return result;
}

auto Device::createRenderPassWithRenderWindow(core::ResourceSharedRef renderWindowRef,
                                              gfx::Color colorClearValue,
                                              real depthClearValue,
                                              uint8_t stencilClearValue) -> core::ResourceUniqueRef
{
    auto result = core::ResourceUniqueRef{};

    auto& resManager = static_cast<resource_manager_t&>(resourceManager());

    auto deviceRef = getSharedFromThis(this);

    result = resManager.allocResource<gfx::RenderPass>(
      deviceRef, renderWindowRef, colorClearValue, depthClearValue, stencilClearValue);

    return result;
}

auto Device::createCommandPool(uint32_t queueFamilyIndex, CommandPoolFlagBits flags) -> core::ResourceUniqueRef
{
    auto result = core::ResourceUniqueRef{};

    auto& resManager = static_cast<resource_manager_t&>(resourceManager());

    auto deviceRef = getSharedFromThis(this);

    result = resManager.allocResource<gfx::CommandPool>(deviceRef, queueFamilyIndex, flags);

    return result;
}

auto Device::createQueueSubmission(QueueSubmissionManager* queueSubmissionManager,
                                   uint32_t queueFamilyIndex,
                                   CommandPoolFlagBits commandPoolFlags) -> core::ResourceUniqueRef
{
    auto result = core::ResourceUniqueRef{};

    auto& resManager = static_cast<resource_manager_t&>(resourceManager());

    auto deviceRef = getSharedFromThis(this);

    result = resManager.allocResource<gfx::QueueSubmission>(
      deviceRef, queueSubmissionManager, queueFamilyIndex, commandPoolFlags);

    return result;
}

auto Device::createDescriptorSetLayout(std::span<Binding const> bindings) -> core::ResourceUniqueRef
{
    auto result = core::ResourceUniqueRef{};

    assert(internalResourceManager_);
    auto* internalResManager = static_cast<internal::internal_resource_manager_t*>(internalResourceManager_.get());

    auto setFlags = VkDescriptorSetLayoutCreateFlags{};
    setFlags = std::accumulate(
      bindings.begin(), bindings.end(), setFlags, [](auto flags, auto const& b) -> VkDescriptorSetLayoutCreateFlags {
          flags |= b.descriptorSetFlags().template cast_to<VkDescriptorSetLayoutCreateFlags>();
          return flags;
      });

    auto bindingFlagVec = std::vector<VkDescriptorBindingFlags>{};
    bindingFlagVec.reserve(bindings.size());
    std::transform(bindings.begin(),
                   bindings.end(),
                   std::back_inserter(bindingFlagVec),
                   [](auto const& binding) -> VkDescriptorBindingFlags {
                       return binding.bindingFlags().template cast_to<VkDescriptorBindingFlags>();
                   });

    auto bindingVec = std::vector<VkDescriptorSetLayoutBinding>{};
    bindingVec.reserve(bindings.size());
    std::transform(bindings.begin(),
                   bindings.end(),
                   std::back_inserter(bindingVec),
                   [](auto const& binding) -> VkDescriptorSetLayoutBinding {
                       auto vkBinding = VkDescriptorSetLayoutBinding{};
                       vkBinding.binding = binding.binding();
                       vkBinding.descriptorType = internal::getDescriptorType(binding.descriptorType());
                       vkBinding.descriptorCount = binding.descriptorCount();
                       vkBinding.stageFlags = binding.stageFlags().template cast_to<VkShaderStageFlags>();

                       return vkBinding;
                   });

    result =
      internalResManager->allocResource<vulkan::DescriptorSetLayout>(handle(), setFlags, bindingFlagVec, bindingVec);

    return result;
}

auto Device::createPipelineBindingSchema(std::span<core::ResourceSharedRef const> setLayouts,
                                         std::span<PushConstantRange const> pushConstantRanges)
  -> core::ResourceUniqueRef
{
    auto result = core::ResourceUniqueRef{};

    auto& resManager = static_cast<resource_manager_t&>(resourceManager());

    auto deviceRef = getSharedFromThis(this);

    result = resManager.allocResource<gfx::PipelineBindingSchema>(deviceRef, setLayouts, pushConstantRanges);

    return result;
}

Device::~Device()
{
    pipelineManager_.reset();

    internalResourceManager_.reset();

    assert(vmaAllocator_ != VK_NULL_HANDLE);
    vmaDestroyAllocator(vmaAllocator_);
}
}

#endif // GFX_DRIVER_VULKAN
