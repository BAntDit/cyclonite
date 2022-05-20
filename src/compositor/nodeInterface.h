//
// Created by anton on 11/25/21.
//
#ifndef CYCLONITE_NODEINTERFACE_H
#define CYCLONITE_NODEINTERFACE_H

#include "vulkan/device.h"
#include <cstddef>
#include <cstdint>

namespace cyclonite::compositor {
class BaseNode;
class FrameCommands;
class Links;

class NodeInterface
{
    using make_expired_func_t = void (*)(void*, size_t);
    using get_expected_wait_signals_func_t = uint32_t (*)();
    using is_surface_node_func_t = bool (*)();
    using begin_func_t = std::pair<VkSemaphore, size_t> (*)(void*, vulkan::Device&, uint64_t);
    using update_func_t = void (*)(void*, uint32_t&, VkSemaphore*, VkPipelineStageFlags* baseFlag);
    using end_func_t = VkSubmitInfo (*)(void*,
                                        VkSemaphore* waitSemaphores,
                                        VkPipelineStageFlags const* waitDstStageMasks,
                                        uint32_t waitSemaphoreCount);
    using get_current_frame_func_t = FrameCommands& (*)(void*);
    using write_frame_commands_func_t = void (*)(void*, vulkan::Device&);
    using dispose_func_t = void (*)(void*);

public:
    NodeInterface(void* node,
                  make_expired_func_t makeExpiredFunc,
                  get_expected_wait_signals_func_t getExpectedWaitSignalsFunc,
                  is_surface_node_func_t isSurfaceNodeFunc,
                  begin_func_t beginFunc,
                  update_func_t updateFunc,
                  end_func_t endFunc,
                  get_current_frame_func_t getFrameFunc,
                  dispose_func_t disposeFunc,
                  write_frame_commands_func_t writeFrameCommandsFunc) noexcept;

    void makeExpired(size_t index);

    auto getCurrentFrame() -> FrameCommands&;

    [[nodiscard]] auto getExpectedWaitSignalCount() const -> uint32_t;

    [[nodiscard]] auto isSurfaceNode() const -> bool;

    auto begin(vulkan::Device& device, uint64_t frameNumber) -> std::pair<VkSemaphore, size_t>;

    void update(uint32_t& signalCount, VkSemaphore* baseSignal, VkPipelineStageFlags* baseFlag);

    auto end(VkSemaphore* waitSemaphores, VkPipelineStageFlags const* waitDstStageMasks, uint32_t waitSemaphoreCount)
      -> VkSubmitInfo;

    void dispose();

    auto get() -> BaseNode&;

    [[nodiscard]] auto get() const -> BaseNode const&;

    auto operator*() -> BaseNode&;

    [[nodiscard]] auto operator*() const -> BaseNode const&;

    auto getRawPtr() -> void* { return node_; }

    [[nodiscard]] auto getRawPtr() const -> void const* { return node_; }

    [[nodiscard]] auto passFinishedSemaphore() const -> vulkan::Handle<VkSemaphore> const&;

    [[nodiscard]] auto getInputs() const -> Links const&;

    auto getInputs() -> Links&;

    void writeFrameCommands(vulkan::Device& device);

private:
    void* node_;
    make_expired_func_t makeExpired_;
    get_expected_wait_signals_func_t getExpectedWaitSignalCount_;
    is_surface_node_func_t isSurfaceNode_;
    begin_func_t begin_;
    update_func_t update_;
    end_func_t end_;
    get_current_frame_func_t getFrame_;
    dispose_func_t dispose_;
    write_frame_commands_func_t writeFrameCommands_;
};
}

#endif // CYCLONITE_NODEINTERFACE_H