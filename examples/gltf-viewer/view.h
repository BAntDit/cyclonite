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

    void init(cyclonite::multithreading::TaskManager& taskManager,
              cyclonite::vulkan::Device& device,
              std::shared_ptr<cyclonite::compositor::Workspace> const&);

    void draw(cyclonite::vulkan::Device& device);

    [[nodiscard]] auto workspace() const -> std::shared_ptr<cyclonite::compositor::Workspace> const&
    {
        return workspace_;
    }

    [[nodiscard]] auto workspace() -> std::shared_ptr<cyclonite::compositor::Workspace>& { return workspace_; }

private:
    std::shared_ptr<cyclonite::compositor::Workspace> workspace_;
};
}

#endif // CYCLONITE_VIEW_H
