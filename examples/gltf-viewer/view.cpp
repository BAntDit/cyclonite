//
// Created by bantdit on 11/8/20.
//

#include "view.h"
#include "model.h"

namespace examples::viewer {
using namespace cyclonite;
using namespace easy_mp;

View::View() noexcept
  : workspace_{ nullptr }
{}

void View::init(std::shared_ptr<compositor::Workspace> const& workspace)
{
    workspace_ = workspace;
}

void View::draw(cyclonite::vulkan::Device& device)
{
    workspace_->render(device);
}
}