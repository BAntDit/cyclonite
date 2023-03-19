//
// Created by bantdit on 11/8/20.
//

#include "view.h"
#include "appConfig.h"
#include "model.h"

namespace examples::viewer {
using namespace cyclonite;
using namespace easy_mp;

View::View() noexcept
  : workspace_{ nullptr }
{
}

void View::init(cyclonite::multithreading::TaskManager& taskManager,
                cyclonite::vulkan::Device& device,
                std::shared_ptr<compositor::Workspace> const& workspace)
{
    workspace_ = workspace;

    {
        auto&& gBufferNode = workspace_->get("g-buffer-node").as(node_type_register_t::node_key_t<GBufferNodeConfig>{});
        gBufferNode.systems().template get<cyclonite::systems::RenderSystem>().init(taskManager, device);
    }

    {
        auto&& surfaceNode = workspace_->get("surface-node").as(node_type_register_t::node_key_t<SurfaceNodeConfig>{});
        surfaceNode.systems().template get<cyclonite::systems::RenderSystem>().init(taskManager, device);
    }
}

void View::draw(cyclonite::vulkan::Device& device)
{
    workspace_->render(device); // TODO:: pass time since last update here
}

void View::dispose()
{
    workspace_.reset();
}
}