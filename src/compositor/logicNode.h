//
// Created by bantdit on 11/9/22.
//

#ifndef CYCLONITE_LOGICNODE_H
#define CYCLONITE_LOGICNODE_H

#include "baseLogicNode.h"
#include "nodeAsset.h"
#include "typedefs.h"

namespace cyclonite::compositor {
/**
 * Logic node class
 */
template<NodeConfig Config>
class alignas(hardware_constructive_interference_size) LogicNode : public BaseLogicNode
{
public:
    using component_config_t = typename Config::component_config_t;
    using systems_config_t = typename Config::systems_config_t;
    using entity_manager_t = enttx::EntityManager<component_config_t>;
    using system_manager_t = enttx::SystemManager<systems_config_t>;
    using asset_t = NodeAsset<component_config_t>;

    LogicNode(resources::ResourceManager& resourceManager, std::string_view name, uint64_t typeId);

    LogicNode(LogicNode const&) = delete;

    LogicNode(LogicNode&&) = default;

    ~LogicNode() = default;

    auto operator=(LogicNode const&) -> LogicNode& = delete;

    auto operator=(LogicNode&&) -> LogicNode& = default;

    [[nodiscard]] auto systems() const -> system_manager_t const& { return systems_; }
    auto systems() -> system_manager_t& { return systems_; }

    [[nodiscard]] auto entities() const -> entity_manager_t const&;
    auto entities() -> entity_manager_t&;

    void update(uint64_t frameNumber, real deltaTime);

private:
    system_manager_t systems_;
};

template<NodeConfig Config>
LogicNode<Config>::LogicNode(resources::ResourceManager& resourceManager, std::string_view name, uint64_t typeId)
  : BaseLogicNode{ resourceManager, name, typeId }
  , systems_{}
{
}

template<NodeConfig Config>
void LogicNode<Config>::update(uint64_t frameNumber, real deltaTime)
{
    auto& a = resourceManager().get(asset()).template as<asset_t>();
    systems_.update(a.entities(), *this, frameNumber, deltaTime);
}

template<NodeConfig Config>
auto LogicNode<Config>::entities() const -> entity_manager_t const&
{
    auto& a = resourceManager().get(asset()).template as<asset_t>();
    return a.entities();
}

template<NodeConfig Config>
auto LogicNode<Config>::entities() -> entity_manager_t&
{
    return const_cast<entity_manager_t&>(std::as_const(*this).entities());
}
}

#endif // CYCLONITE_LOGICNODE_H
