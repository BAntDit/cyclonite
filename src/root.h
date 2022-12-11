//
// Created by bantdit on 9/8/19.
//

#ifndef CYCLONITE_ROOT_H
#define CYCLONITE_ROOT_H

#include "compositor/workspace.h"
#include "config.h"
#include "input.h"
#include "multithreading/taskManager.h"
#include "platform.h"
#include "resources/resourceManager.h"
#include "sdl/sdlSupport.h"
#include "surface.h"
#include "vulkan/device.h"
#include "vulkan/instance.h"

#include <iostream>
#include <memory>

namespace cyclonite {
class Root
{
public:
    struct Capabilities
    {
        std::vector<std::pair<uint16_t, uint16_t>> displayResolutions;
    };

public:
    Root();

    Root(Root const&) = delete;

    Root(Root&&) = delete;

    ~Root() = default;

    auto operator=(Root const&) -> Root& = delete;

    auto operator=(Root &&) -> Root& = delete;

    void init();

    void init(uint32_t deviceId);

    void declareResources(size_t initialResourceCount, resources::ResourceRegInfoSpecialization auto&&... regInfo);

    [[nodiscard]] auto getDeviceCount() const -> size_t { return physicalDeviceList_.size(); }

    [[nodiscard]] auto getDeviceId(size_t deviceIndex = 0) const -> uint32_t;

    [[nodiscard]] auto getDeviceId(std::string const& deviceName) const -> uint32_t;

    [[nodiscard]] auto capabilities() const -> Capabilities const& { return capabilities_; }

    [[nodiscard]] auto input() const -> Input const& { return input_; }

    [[nodiscard]] auto input() -> Input& { return input_; }

    [[nodiscard]] auto device() const -> vulkan::Device const& { return *vulkanDevice_; }

    [[nodiscard]] auto device() -> vulkan::Device& { return *vulkanDevice_; }

    [[nodiscard]] auto taskManager() -> multithreading::TaskManager& { return taskManager_; }

    [[nodiscard]] auto taskManager() const -> multithreading::TaskManager const& { return taskManager_; }

    [[nodiscard]] auto resourceManager() -> resources::ResourceManager&;

    [[nodiscard]] auto resourceManager() const -> resources::ResourceManager const&;

    template<typename WorkspaceFactory>
    auto createWorkspace(WorkspaceFactory&& workspaceFactory) -> std::shared_ptr<compositor::Workspace> const&;

    void dispose();

private:
    Capabilities capabilities_;
    std::unique_ptr<resources::ResourceManager> resourceManager_;
    multithreading::TaskManager taskManager_;
    sdl::SDLSupport sdlSupport_;
    std::unique_ptr<vulkan::Instance> vulkanInstance_;
    std::vector<VkPhysicalDevice> physicalDeviceList_;
    std::unordered_map<std::string, VkPhysicalDeviceProperties> physicalDevicePropertiesMap_;
    std::unique_ptr<vulkan::Device> vulkanDevice_;
    std::vector<std::shared_ptr<compositor::Workspace>> workspaces_;
    Input input_;
};

template<typename WorkspaceFactory>
auto Root::createWorkspace(WorkspaceFactory&& workspaceFactory) -> std::shared_ptr<compositor::Workspace> const&
{
    return workspaces_.emplace_back(std::make_shared<compositor::Workspace>(
      workspaceFactory(compositor::Workspace::Builder{ *resourceManager_, *vulkanDevice_ })));
}

void Root::declareResources(size_t initialResourceCount, resources::ResourceRegInfoSpecialization auto&&... regInfo)
{
    assert(!resourceManager_);

    resourceManager_ = std::make_unique<resources::ResourceManager>(initialResourceCount);
    resourceManager_->template registerResources(std::forward<decltype(regInfo)>(regInfo)...);
}
}
#endif // CYCLONITE_ROOT_H
