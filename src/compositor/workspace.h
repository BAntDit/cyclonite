//
// Created by anton on 12/19/20.
//

#ifndef CYCLONITE_WORKSPACE_H
#define CYCLONITE_WORKSPACE_H

#include "baseNode.h"

namespace cyclonite::compositor {
class Workspace
{
public:
    void render(vulkan::Device& device);

public:
    class Builder
    {
    public:
        explicit Builder(vulkan::Device& device);

        template<typename NodeFactory>
        auto createNode(NodeFactory&& nodeFactory) -> Builder&;

        auto build() -> Workspace;

    private:
        vulkan::Device* device_;
        std::vector<vulkan::Handle<VkSemaphore>> nodeSignalSemaphores_;
        std::vector<VkSemaphore> nodeWaitSemaphores_;
        std::vector<uint32_t> waitSemaphoresPerNodeCount_;
        std::vector<std::shared_ptr<BaseNode>> nodes_;
    };

private:
    uint32_t frameNumber_;
    uint32_t frameIndex_;
    uint32_t swapChainLength_;

    std::vector<std::shared_ptr<BaseNode>> nodes_;
    std::vector<vulkan::Handle<VkFence>> frameFences_;
};

template<typename NodeFactory>
auto Workspace::Builder::createNode(NodeFactory&& nodeFactory) -> Workspace::Builder&
{
    static_assert(std::is_invocable_v<decltype(nodeFactory), vulkan::Device&>);

    nodes_.emplace_back(nodeFactory(*device_));

    return *this;
}
}

#endif // CYCLONITE_WORKSPACE_H
