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
    Builder(resources::ResourceManager& resourceManager,
            T* wsBuilder,
            uint64_t (M::*nameToId)(std::string_view),
            uint64_t typeId);

    auto setName(std::string_view name) -> Builder&;

    auto addDependency(std::string_view name) -> Builder&;

    auto build() -> LogicNode<Config>;

private:
    resources::ResourceManager* resourceManager_;
    std::string name_;
    std::function<uint64_t(std::string_view)> nameToNodeId_;
    std::vector<std::string> dependencies_;
    uint64_t nodeTypeId_;
};

template<NodeConfig Config>
template<typename T, typename M>
BaseLogicNode::Builder<Config>::Builder(resources::ResourceManager& resourceManager,
                                        T* wsBuilder,
                                        uint64_t (M::*nameToId)(std::string_view),
                                        uint64_t typeId)
  : resourceManager_{ &resourceManager }
  , name_{}
  , nameToNodeId_{ [wsBuilder, nameToId](std::string_view n) -> uint64_t { return wsBuilder->*nameToId(n); } }
  , dependencies_{}
  , nodeTypeId_{ typeId }
{}

template<NodeConfig Config>
auto BaseLogicNode::Builder<Config>::setName(std::string_view name) -> Builder&
{
    name_ = name;
    return *this;
}

template<NodeConfig Config>
auto BaseLogicNode::Builder<Config>::addDependency(std::string_view name) -> Builder&
{
    dependencies_.emplace_back(name);
}

template<NodeConfig Config>
auto BaseLogicNode::Builder<Config>::build() -> LogicNode<Config>
{
    auto node = LogicNode<Config>{ *resourceManager_, std::string_view{ name_.data(), name_.size() }, nodeTypeId_ };

    for (auto&& dep : dependencies_) {
        node.dependencies_.insert(std::pair{ nameToNodeId_(dep), std::nullopt });
    }

    return node;
}
}

#endif // CYCLONITE_LOGICNODEBUILDER_H
