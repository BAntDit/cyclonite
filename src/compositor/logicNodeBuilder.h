//
// Created by bantdit on 11/18/22.
//

#ifndef CYCLONITE_LOGICNODEBUILDER_H
#define CYCLONITE_LOGICNODEBUILDER_H

#include "logicNode.h"
#include <enttx/entity.h>

namespace cyclonite::compositor {
template<NodeConfig Config>
class BaseLogicNode::Builder
{
public:
    template<typename T, typename M>
    Builder(resources::ResourceManager& resourceManager, T* wsBuilder, uint64_t (M::*indexToId)(size_t));

    void setName(std::string_view name) { name_ = name; }

    auto setDependencies(std::initializer_list<size_t> const& indices) -> Builder&;

    auto build() -> LogicNode<Config>;

private:
    resources::ResourceManager* resourceManager_;
    std::string name_;
    std::function<uint64_t(size_t)> indexToNodeId_;
    std::vector<size_t> dependencies_;
};

template<NodeConfig Config>
template<typename T, typename M>
BaseLogicNode::Builder<Config>::Builder(resources::ResourceManager& resourceManager,
                                        T* wsBuilder,
                                        uint64_t (M::*indexToId)(size_t))
  : resourceManager_{ &resourceManager }
  , indexToNodeId_{ [wsBuilder, indexToId](size_t i) -> uint64_t { return wsBuilder->*indexToId(i); } }
  , dependencies_{}
{}

template<NodeConfig Config>
auto BaseLogicNode::Builder<Config>::setDependencies(std::initializer_list<size_t> const& indices) -> Builder<Config>&
{
    dependencies_ = std::vector<size_t>{ indices };
}

template<NodeConfig Config>
auto BaseLogicNode::Builder<Config>::build() -> LogicNode<Config>
{
    auto&& node = LogicNode<Config>{ *resourceManager_, std::string_view{ name_.data(), name_.size() } };

    for (auto&& dep : dependencies_) {
        node.dependencies_.insert(std::pair{ indexToNodeId_(dep), std::nullopt });
    }

    return node;
}
}

#endif // CYCLONITE_LOGICNODEBUILDER_H
