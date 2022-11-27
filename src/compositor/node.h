//
// Created by anton on 3/21/21.
//

#ifndef CYCLONITE_NODE_H
#define CYCLONITE_NODE_H

#include "config.h"
#include "nodeIdentifier.h"
#include "nodeTypeRegister.h"
#include "resources/resource.h"
#include <future>
#include <optional>
#include <unordered_map>

namespace cyclonite::compositor {
class Node : public NodeIdentifier
{
public:
    Node(resources::ResourceManager& resourceManager, std::string_view name, [[maybe_unused]] uint64_t typeId) noexcept;

    [[nodiscard]] auto scene() const -> resources::Resource::Id { return scene_; }
    auto scene() -> resources::Resource::Id& { return scene_; }

    [[nodiscard]] auto resourceManager() const -> resources::ResourceManager const& { return *resourceManager_; }
    auto resourceManager() -> resources::ResourceManager& { return *resourceManager_; }

    [[nodiscard]] auto typeId() const -> uint64_t
    {
#if !defined(NDEBUG)
        return typeId_;
#else
        return std::numeric_limits<uint64_t>::max();
#endif
    }

    template<NodeConfig Config, typename NodeTypeId>
    [[nodiscard]] auto as(type_pair<node_t<Config>, NodeTypeId>) const -> node_t<Config> const&;

    template<NodeConfig Config, typename NodeTypeId>
    auto as(type_pair<node_t<Config>, NodeTypeId>) -> node_t<Config>&;

    auto dependsOn(uint64_t id) const -> bool { return dependencies_.contains(id); }

    void updateDependency(uint64_t id, std::shared_future<void> const& dependency);

    void resolveDependencies();

protected:
    std::unordered_map<uint64_t, std::optional<std::shared_future<void>>> dependencies_;

private:
    resources::ResourceManager* resourceManager_;
    resources::Resource::Id scene_;
#if !defined(NDEBUG)
    uint64_t typeId_;
#endif
};

template<NodeConfig Config, typename NodeTypeId>
auto Node::as(type_pair<node_t<Config>, NodeTypeId>) const -> node_t<Config> const&
{
#if !defined(NDEBUG)
    assert(typeId_ == NodeTypeId::value);
#endif
    return *(reinterpret_cast<node_t<Config> const*>(this));
}

template<NodeConfig Config, typename NodeTypeId>
auto Node::as(type_pair<node_t<Config>, NodeTypeId>) -> node_t<Config>&
{
#if !defined(NDEBUG)
    assert(typeId_ == NodeTypeId::value);
#endif
    return *(reinterpret_cast<node_t<Config>*>(this));
}
}

#endif // CYCLONITE_NODE_H
