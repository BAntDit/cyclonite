//
// Created by bantdit on 11/9/22.
//

#include "logicNodeInterface.h"
#include "baseLogicNode.h"

namespace cyclonite::compositor {
LogicNodeInterface::LogicNodeInterface(void* node, update_func_t updateFunc, dispose_func_t disposeFunc)
  : node_{ node }
  , update_{ updateFunc }
  , dispose_{ disposeFunc }
{}

auto LogicNodeInterface::get() -> BaseLogicNode&
{
    return *reinterpret_cast<BaseLogicNode*>(node_);
}

auto LogicNodeInterface::get() const -> BaseLogicNode const&
{
    return *reinterpret_cast<BaseLogicNode const*>(node_);
}

auto LogicNodeInterface::operator*() -> BaseLogicNode&
{
    return *reinterpret_cast<BaseLogicNode*>(node_);
}

auto LogicNodeInterface::operator*() const -> BaseLogicNode const&
{
    return *reinterpret_cast<BaseLogicNode const*>(node_);
}

void LogicNodeInterface::update(uint32_t frameIndex, real deltaTime)
{
    update_(node_, frameIndex, deltaTime);
}

void LogicNodeInterface::dispose()
{
    dispose_(node_);
}
}