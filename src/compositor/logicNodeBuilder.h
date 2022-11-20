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
    Builder() = default;

    auto setCamera(enttx::Entity camera) -> Builder& { camera_ = static_cast<uint64_t>(camera); }

    auto build() -> LogicNode<Config>;

private:
    uint64_t camera_;
};

template<NodeConfig Config>
auto BaseLogicNode::Builder<Config>::build() -> LogicNode<Config>
{
    auto node = LogicNode<Config>{};
    node.camera() = camera_;

    return node;
}
}

#endif // CYCLONITE_LOGICNODEBUILDER_H
