//
// Created by anton on 2/28/21.
//

#include "workspace.h"

namespace cyclonite::compositor {
static bool testDependency(BaseNode const& a, BaseNode const& b);

static void writeNodeSignals(vulkan::Device const& device,
                             std::vector<BaseNode> const& nodes,
                             size_t index,
                             std::vector<vulkan::Handle<VkSemaphore>>& toSignal,
                             std::vector<VkSemaphore>& toWait,
                             std::vector<uint32_t>& waitSignalsPerNode);

Workspace::Builder::Builder(vulkan::Device& device)
  : device_{ &device }
  , nodeSignalSemaphores_{}
  , nodeWaitSemaphores_{}
  , waitSemaphoresPerNodeCount_{}
  , nodes_{}
{}

auto Workspace::Builder::build() -> Workspace {}

static bool testDependency(BaseNode const& a, BaseNode const& b)
{
    auto const& inputs = a.getInputs();
    auto const& outputs = b.getOutputs();

    for (auto i = size_t{ 0 }, iCount = inputs.size(); i < iCount; i++) {
        auto const& input = inputs.get(i);

        for (auto j = size_t{ 0 }, jCount = inputs.size(); j < jCount; j++) {
            auto const& output = outputs.get(j);

            if (input == output)
                return true;
        }
    }

    return false;
}

static void writeNodeSignals(vulkan::Device const& device,
                             std::vector<BaseNode> const& nodes,
                             size_t index,
                             std::vector<vulkan::Handle<VkSemaphore>>& toSignal,
                             std::vector<VkSemaphore>& toWait,
                             std::vector<uint32_t>& waitSignalsPerNode)
{
    assert(index < nodes.size());

    auto const& node = nodes[index];

    {
        auto semaphoreCreateInfo = VkSemaphoreCreateInfo{};
        semaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

        if (auto result = vkCreateSemaphore(device.handle(),
                                            &semaphoreCreateInfo,
                                            nullptr,
                                            &toSignal.emplace_back(device.handle(), vkDestroySemaphore));
            result != VK_SUCCESS) {
            throw std::runtime_error("could not create node signal semaphore.");
        }
    }

    auto& waitSemaphoresCount = waitSignalsPerNode.emplace_back(0);

    for (auto i = size_t{ 0 }, count = nodes.size(); i < count; i++) {
        auto const& n = nodes[i];

        if (testDependency(node, n)) {
            toWait.emplace_back(static_cast<VkSemaphore>(toSignal[i]));
            waitSemaphoresCount++;
        }
    }
}
}
