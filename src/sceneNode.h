//
// Created by bantdit on 1/19/20.
//

#ifndef CYCLONITE_SCENENODE_H
#define CYCLONITE_SCENENODE_H

#include <enttx/enttx.h>
#include <memory>

namespace cyclonite {
class Scene;

class SceneNodeChildren;

class SceneNode
{
public:
    SceneNode(SceneNode const&) = default;

    SceneNode(SceneNode&&) = default;

    ~SceneNode() = default;

    auto operator=(SceneNode const&) -> SceneNode& = default;

    auto operator=(SceneNode &&) -> SceneNode& = default;

private:
    SceneNode(std::shared_ptr<Scene> const& scene,
              enttx::Entity parent,
              enttx::Entity entity,
              size_t firstChildrenIndex,
              size_t childCount);

private:
    std::weak_ptr<Scene> scene_;
    enttx::Entity parent_;
    enttx::Entity entity_;

    size_t firstChildIndex_;
    size_t childCount_;
};

class SceneNodeChildren
{
public:
    class Iterator
    {
        using iterator_category = std::input_iterator_tag;
        using value_type = SceneNode;
        using difference_type = uint32_t;
        using pointer = value_type*;
        using reference = value_type&;

    private:
        friend class Children;
    };
};
}

#endif // CYCLONITE_SCENENODE_H
