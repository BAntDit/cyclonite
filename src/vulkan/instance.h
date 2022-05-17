//
// Created by bantdit on 9/1/19.
//

#ifndef CYCLONITE_INSTANCE_H
#define CYCLONITE_INSTANCE_H

#include "handle.h"
#include <array>
#include <cstdint>
#include <iostream>
#include <sstream>

namespace cyclonite::vulkan {
class Instance
{
public:
    template<size_t N, size_t M>
    Instance(std::array<char const*, N> const& reqLayers, std::array<char const*, M> const& reqExtensions);

    Instance(Instance const&) = delete;

    Instance(Instance&&) = default;

    ~Instance() = default;

    auto operator=(Instance const&) -> Instance& = delete;

    auto operator=(Instance &&) -> Instance& = default;

public:
    [[nodiscard]] auto handle() const -> VkInstance { return static_cast<VkInstance>(vkInstance_); }

private:
    template<size_t N>
    void testLayers(std::array<char const*, N> const& reqLayers);

    template<size_t N>
    void testExtensions(std::array<char const*, N> const& reqExtensions);

    void createInstance(uint32_t layerCount,
                        char const* const* layerNames,
                        uint32_t extensionCount,
                        char const* const* extensionNames);

private:
    Handle<VkInstance> vkInstance_;
};

template<size_t N, size_t M>
Instance::Instance(std::array<char const*, N> const& reqLayers, std::array<char const*, M> const& reqExtensions)
  : vkInstance_{ vkDestroyInstance }
{
    testLayers(reqLayers);

    testExtensions(reqExtensions);

    createInstance(static_cast<uint32_t>(N),
                   N > 0 ? reqLayers.data() : nullptr,
                   static_cast<uint32_t>(M),
                   M > 0 ? reqExtensions.data() : nullptr);
}

template<size_t N>
void Instance::testLayers(std::array<char const*, N> const& reqLayers)
{
    uint32_t availableInstanceLayerCount = 0;

    if (vkEnumerateInstanceLayerProperties(&availableInstanceLayerCount, nullptr) != VK_SUCCESS) {
        throw std::runtime_error("could not enumerate available instance layers");
    }

    std::vector<VkLayerProperties> availableLayers{};

    if (availableInstanceLayerCount > 0) {
        availableLayers.resize(availableInstanceLayerCount);

        if (vkEnumerateInstanceLayerProperties(&availableInstanceLayerCount, availableLayers.data()) != VK_SUCCESS) {
            throw std::runtime_error("could not read available instance layer properties");
        }
    }

    for (auto layerIt = reqLayers.cbegin(); layerIt != reqLayers.cend(); ++layerIt) {
        auto it = availableLayers.cbegin();

        while (it != availableLayers.cend()) {
            if (0 == strcmp((*layerIt), (*it).layerName))
                break;
            it++;
        }

        if (it == availableLayers.cend())
            throw std::runtime_error("required layer: " + std::string(*layerIt) + " is not supported");
    }
}

template<size_t N>
void Instance::testExtensions(std::array<char const*, N> const& reqExtensions)
{
    uint32_t availableInstanceExtensionsCount = 0;

    if (vkEnumerateInstanceExtensionProperties(nullptr, &availableInstanceExtensionsCount, nullptr) != VK_SUCCESS) {
        throw std::runtime_error("could not enumerate available instance extensions");
    }

    std::vector<VkExtensionProperties> availableExtensions{};

    if (availableInstanceExtensionsCount > 0) {
        availableExtensions.resize(availableInstanceExtensionsCount);

        if (vkEnumerateInstanceExtensionProperties(
              nullptr, &availableInstanceExtensionsCount, availableExtensions.data()) != VK_SUCCESS) {
            throw std::runtime_error("count not read properties of available extensions");
        }
    }

    for (auto extIt = reqExtensions.cbegin(); extIt != reqExtensions.cend(); ++extIt) {
        auto it = availableExtensions.cbegin();

        while (it != availableExtensions.cend()) {
            if (0 == strcmp((*extIt), (*it).extensionName))
                break;
            it++;
        }

        if (it == availableExtensions.cend()) {
            throw std::runtime_error("required extension: " + std::string(*extIt) + " is not supported");
        }
    }
}
}

#endif // CYCLONITE_INSTANCE_H
