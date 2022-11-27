//
// Created by bantdit on 11/27/22.
//

#include "node.h"

namespace cyclonite::compositor {
Node::Node(resources::ResourceManager& resourceManager,
           std::string_view name,
           [[maybe_unused]] uint64_t typeId) noexcept
  : NodeIdentifier{ name }
  , dependencies_{}
  , resourceManager_{ &resourceManager }
  , scene_
{}
#if !defined(NDEBUG)
, typeId_
{
    typeId
}
#endif
{}

void Node::resolveDependencies()
{
    for (auto&& [_, dep] : dependencies_) {
        (void)_;
        assert(dep);

        dep->get();
    }
}

void Node::updateDependency(uint64_t id, std::shared_future<void> const& dependency)
{
    assert(dependencies_.contains(id));
    dependencies_[id] = dependency;
}
}