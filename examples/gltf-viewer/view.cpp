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
{}

void View::init(cyclonite::multithreading::TaskManager& taskManager,
                cyclonite::vulkan::Device& device,
                std::shared_ptr<compositor::Workspace> const& workspace)
{
    workspace_ = workspace;

    {
        // auto& node = workspace_->get(node_type_register_t::node_key_t<MainNodeConfig>{});
        // node.systems().get<systems::RenderSystem>().init(taskManager, device);
    }

    {
        // auto& node = workspace_->get(node_type_register_t::node_key_t<SurfaceNodeConfig>{});
        // node.systems().get<systems::RenderSystem>().init(taskManager, device);
    }
}

void View::draw(cyclonite::vulkan::Device& device)
{
    workspace_->render(device);
}

void View::dispose()
{
    workspace_.reset();
}
}