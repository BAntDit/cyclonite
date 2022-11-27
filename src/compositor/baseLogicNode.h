//
// Created by bantdit on 11/9/22.
//

#ifndef CYCLONITE_BASELOGICNODE_H
#define CYCLONITE_BASELOGICNODE_H

#include "config.h"
#include "nodeIdentifier.h"
#include "resources/resource.h"
#include <cstdint>
#include <unordered_map>

namespace cyclonite::compositor {
class BaseLogicNode : public NodeIdentifier
{
public:
    BaseLogicNode(resources::ResourceManager& resourceManager, std::string_view name) noexcept;

    [[nodiscard]] auto scene() const -> resources::Resource::Id { return scene_; }
    auto scene() -> resources::Resource::Id& { return scene_; }

    [[nodiscard]] auto resourceManager() const -> resources::ResourceManager const& { return *resourceManager_; }
    auto resourceManager() -> resources::ResourceManager& { return *resourceManager_; }

    void waitForDependencies();

    auto dependsOn(uint64_t id) const -> bool { return dependencies_.contains(id); }

    void updateDependency(uint64_t id, std::shared_future<void> const& dependency);

public:
    template<NodeConfig Config>
    class Builder;

private:
    std::unordered_map<uint64_t, std::optional<std::shared_future<void>>> dependencies_;
    resources::ResourceManager* resourceManager_;
    resources::Resource::Id scene_;
};
}

#endif // CYCLONITE_BASELOGICNODE_H
