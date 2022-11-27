//
// Created by bantdit on 11/9/22.
//

#ifndef CYCLONITE_LOGICNODEINTERFACE_H
#define CYCLONITE_LOGICNODEINTERFACE_H

#include "typedefs.h"
#include <cstdint>

namespace cyclonite::compositor {
class BaseLogicNode;

class LogicNodeInterface
{
public:
    using update_func_t = void (*)(void*, uint32_t, real);
    using dispose_func_t = void (*)(void*);

    LogicNodeInterface(void* node, update_func_t updateFunc, dispose_func_t disposeFunc);

    LogicNodeInterface(LogicNodeInterface const&) = default;

    LogicNodeInterface(LogicNodeInterface&&) = default;

    ~LogicNodeInterface() = default;

    auto operator=(LogicNodeInterface const&) -> LogicNodeInterface& = default;

    auto operator=(LogicNodeInterface &&) -> LogicNodeInterface& = default;

    auto get() -> BaseLogicNode&;

    [[nodiscard]] auto get() const -> BaseLogicNode const&;

    auto operator*() -> BaseLogicNode&;

    [[nodiscard]] auto operator*() const -> BaseLogicNode const&;

    void update(uint32_t frameNumber, real deltaTime);

    void dispose();

private:
    void* node_;
    update_func_t update_;
    dispose_func_t dispose_;
};
}

#endif // CYCLONITE_LOGICNODEINTERFACE_H
