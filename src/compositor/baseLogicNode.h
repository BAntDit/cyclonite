//
// Created by bantdit on 11/9/22.
//

#ifndef CYCLONITE_BASELOGICNODE_H
#define CYCLONITE_BASELOGICNODE_H

#include "node.h"
#include <cstdint>

namespace cyclonite::compositor {
class BaseLogicNode : public Node
{
public:
    BaseLogicNode(resources::ResourceManager& resourceManager, std::string_view name, uint64_t typeId) noexcept;

public:
    template<NodeConfig Config>
    class Builder;
};
}

#endif // CYCLONITE_BASELOGICNODE_H
