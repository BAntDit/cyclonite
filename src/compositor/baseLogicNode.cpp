//
// Created by bantdit on 11/26/22.
//

#include "baseLogicNode.h"

namespace cyclonite::compositor {
BaseLogicNode::BaseLogicNode(resources::ResourceManager& resourceManager,
                             std::string_view name,
                             uint64_t typeId) noexcept
  : Node{ resourceManager, name, typeId }
{}
}