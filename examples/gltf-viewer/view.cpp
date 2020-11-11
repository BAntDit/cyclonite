//
// Created by bantdit on 11/8/20.
//

#include "view.h"
#include "model.h"

namespace examples::viewer {
using namespace cyclonite;

View::View() noexcept
  : systems_{ nullptr }
{}

void View::init(cyclonite::vulkan::Device& device,
                cyclonite::multithreading::TaskManager& taskManager,
                ecs_config_t::system_manager_t& systems)
{
    systems_ = &systems;

    auto windowProperties = WindowProperties{};
    windowProperties.title = "gltf-viewer.example";
    windowProperties.fullscreen = false;
    windowProperties.left = 0;
    windowProperties.top = 0;
    windowProperties.width = 512;
    windowProperties.height = 512;

    auto& renderSystem = systems.get<systems::RenderSystem>();
    renderSystem.init(
      taskManager,
      device,
      windowProperties,
      render_target_output<type_list<render_target_output_candidate<VK_FORMAT_D32_SFLOAT>>>{},
      render_target_output<
        type_list<render_target_output_candidate<VK_FORMAT_B8G8R8A8_UNORM, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR>>,
        RenderTargetOutputSemantic::DEFAULT>{},
      VkClearDepthStencilValue{ 1.0f, 0 },
      VkClearColorValue{ { 0.188f, 0.835f, 0.784f } },
      std::array{ VK_PRESENT_MODE_MAILBOX_KHR, VK_PRESENT_MODE_FIFO_KHR });

    auto& uniformSystem = systems.get<systems::UniformSystem>();
    uniformSystem.init(device, renderSystem.renderPass().getSwapChainLength());
}

void View::draw(Model const& model)
{
    systems_->update(model.camera(), model.timeSinceLastUpdate());
}
}