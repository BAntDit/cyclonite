//
// Created by bantdit on 11/23/22.
//

#ifndef CYCLONITE_NODEASSET_H
#define CYCLONITE_NODEASSET_H

#include "config.h"
#include "resources/resource.h"
#include <metrix/type_list.h>
#include <enttx/enttx.h>

namespace cyclonite::compositor {
namespace internal {
template<ComponentConfig ComponentCfg>
struct get_entity_manager_config;

template<typename... C, typename... S>
struct get_entity_manager_config<component_config_t<type_list<C...>, type_list<S...>>>
{
    using type = enttx::EntityManagerConfig<type_list<C...>, type_list<S...>>;
};

template<ComponentConfig ComponentCfg>
using entity_manager_config_t = typename get_entity_manager_config<ComponentCfg>::type;

template<ComponentConfig ComponentCfg>
using entity_manager_t = enttx::EntityManager<entity_manager_config_t<ComponentCfg>>;
}

template<ComponentConfig ComponentCfg>
class NodeAsset : public resources::Resource
{
public:
    using component_config_t = ComponentCfg;
    using entity_manager_t = internal::entity_manager_t<ComponentCfg>;

    [[nodiscard]] auto entities() const -> entity_manager_t const& { return entityManager_; }
    auto entities() -> entity_manager_t& { return entityManager_; }

    [[nodiscard]] auto instance_tag() const -> ResourceTag const& override { return tag; }

private:
    entity_manager_t entityManager_;

private:
    static ResourceTag tag;

public:
    static auto type_tag_const() -> ResourceTag const& { return NodeAsset::tag; }
    static auto type_tag() -> ResourceTag& { return NodeAsset::tag; }
};

template<ComponentConfig ComponentCfg>
resources::Resource::ResourceTag NodeAsset<ComponentCfg>::tag{};
}

#endif // CYCLONITE_NODEASSET_H
