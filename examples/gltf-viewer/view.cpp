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

void View::init(compositor::Workspace const& workspace)
{
    (void)workspace;
    // workspace_ = workspace;
}

void View::draw() {}
}