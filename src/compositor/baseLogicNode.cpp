//
// Created by bantdit on 11/26/22.
//

#include "baseLogicNode.h"

namespace cyclonite::compositor {
BaseLogicNode::BaseLogicNode(resources::ResourceManager& resourceManager, std::string_view name) noexcept
  : NodeIdentifier{ name }
  , dependencies_{}
  , resourceManager_{ &resourceManager }
  , scene_{}
{}

void BaseLogicNode::resolveDependencies()
{
    for (auto&& [_, dep] : dependencies_) {
        (void)_;
        assert(dep);

        dep->get();
    }
}

void BaseLogicNode::updateDependency(uint64_t id, std::shared_future<void> const& dependency)
{
    assert(dependencies_.contains(id));
    dependencies_[id] = dependency;
}
}