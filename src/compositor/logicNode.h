//
// Created by bantdit on 11/9/22.
//

#ifndef CYCLONITE_LOGICNODE_H
#define CYCLONITE_LOGICNODE_H

#include "baseLogicNode.h"
#include "typedefs.h"

namespace cyclonite::compositor {
template<NodeConfig Config>
class alignas(hardware_constructive_interference_size) LogicNode : public BaseLogicNode
{
public:
    using ecs_config_t = typename config_traits::get_ecs_config_t<Config>;
    using node_component_list_t = typename ecs_config_t::component_list_t;
    using node_storage_list_t = typename ecs_config_t::storage_list_t;
    using node_system_list_t = typename ecs_config_t::system_list_t;

    using entity_manager_t = typename ecs_config_t::entity_manager_t;
    using system_manager_t = typename ecs_config_t::system_manager_t;

    LogicNode() = default;

    LogicNode(LogicNode const&) = delete;

    LogicNode(LogicNode&&) = default;

    ~LogicNode() = default;

    auto operator=(LogicNode const&) -> LogicNode& = delete;

    auto operator=(LogicNode &&) -> LogicNode& = default;

    auto systems() -> system_manager_t& { return systems_; }

    [[nodiscard]] auto systems() const -> system_manager_t const& { return systems_; }

    auto entities() -> entity_manager_t& { return entities_; }

    [[nodiscard]] auto entities() const -> entity_manager_t const& { return entities_; }

    void update(uint32_t frameIndex, real deltaTime);

    void dispose()
    {
        // TODO:: ...
    }

private:
    entity_manager_t entities_;
    system_manager_t systems_;
    uint64_t camera_;
};

template<NodeConfig Config>
void LogicNode<Config>::update(uint32_t frameIndex, real deltaTime)
{
    systems_.update(this, frameIndex, deltaTime);
}
}

#endif // CYCLONITE_LOGICNODE_H
