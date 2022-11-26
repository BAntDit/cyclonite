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
}