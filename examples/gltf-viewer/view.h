//
// Created by bantdit on 11/2/20.
//

#ifndef CYCLONITE_VIEW_H
#define CYCLONITE_VIEW_H

#include "viewer.h"

namespace examples::viewer {
class Model;

class View
{
public:
    View() noexcept;

    void init(std::shared_ptr<cyclonite::compositor::Workspace> const&);

    void draw(cyclonite::vulkan::Device& device);

private:
    std::shared_ptr<cyclonite::compositor::Workspace> workspace_;
};
}

#endif // CYCLONITE_VIEW_H
