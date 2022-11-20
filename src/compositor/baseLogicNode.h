//
// Created by bantdit on 11/9/22.
//

#ifndef CYCLONITE_BASELOGICNODE_H
#define CYCLONITE_BASELOGICNODE_H

#include "config.h"
#include <cstdint>

namespace cyclonite::compositor {
class BaseLogicNode
{
public:
    BaseLogicNode() = default;

    [[nodiscard]] auto camera() const -> uint64_t { return camera_; }

public:
    template<NodeConfig Config>
    class Builder;

protected:
    auto camera() -> uint64_t& { return camera_; }

private:
    uint64_t camera_;
};
}

#endif // CYCLONITE_BASELOGICNODE_H
