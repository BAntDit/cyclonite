//
// Created by bantdit on 11/9/22.
//

#ifndef CYCLONITE_LOGICNODE_H
#define CYCLONITE_LOGICNODE_H

#include "baseLogicNode.h"
#include "scene.h"
#include "typedefs.h"

namespace cyclonite::compositor {
template<NodeConfig Config>
class alignas(hardware_constructive_interference_size) LogicNode : public BaseLogicNode
{
public:
    using component_config_t = typename Config::component_config_t;
    using systems_config_t = typename Config::systems_config_t;
    using entity_manager_t = enttx::EntityManager<component_config_t>;
    using system_manager_t = enttx::SystemManager<systems_config_t>;
    using scene_t = Scene<component_config_t>;

    LogicNode(resources::ResourceManager& resourceManager, std::string_view name);

    LogicNode(LogicNode const&) = delete;

    LogicNode(LogicNode&&) = default;

    ~LogicNode() = default;

    auto operator=(LogicNode const&) -> LogicNode& = delete;

    auto operator=(LogicNode &&) -> LogicNode& = default;

    [[nodiscard]] auto systems() const -> system_manager_t const& { return systems_; }
    auto systems() -> system_manager_t& { return systems_; }

    [[nodiscard]] auto entities() const -> entity_manager_t const&;
    auto entities() -> entity_manager_t&;

    void update(uint32_t frameIndex, real deltaTime);

    void dispose()
    {
        // TODO:: ...
    }

private:
    system_manager_t systems_;
};

template<NodeConfig Config>
LogicNode<Config>::LogicNode(resources::ResourceManager& resourceManager, std::string_view name)
  : BaseLogicNode{ resourceManager, name }
  , systems_{}
{}

template<NodeConfig Config>
void LogicNode<Config>::update(uint32_t frameIndex, real deltaTime)
{
    auto& s = resourceManager().get(scene()).template as<scene_t>();
    systems_.update(s.entities(), this, frameIndex, deltaTime);
}

template<NodeConfig Config>
auto LogicNode<Config>::entities() const -> entity_manager_t const&
{
    auto& s = resourceManager().get(scene()).template as<scene_t>();
    return s.entities();
}

template<NodeConfig Config>
auto LogicNode<Config>::entities() -> entity_manager_t&
{
    return const_cast<resources::ResourceManager&>(std::as_const(this)->entities());
}
}

#endif // CYCLONITE_LOGICNODE_H
