//
// Created by bantdit on 9/8/19.
//

#ifndef CYCLONITE_ROOT_H
#define CYCLONITE_ROOT_H

#include "resources/resourceGroupManager.h"
#include "rootBase.h"
#include "rootConfigTraits.h"

namespace cyclonite {
namespace internal {
struct DefaultConfig
{};
};

template<typename Config = internal::DefaultConfig>
class Root : public RootBase
{
public:
    using config_t = cyclonite::ConfigTraits<Config>;

    Root();

    Root(Root const&) = delete;

    Root(Root&&) = delete;

    auto operator=(Root const&) -> Root& = delete;

    auto operator=(Root&&) -> Root& = delete;

private:
    resources::ResourceGroupManager<Config> resourceGroupManager_;

public:
    [[nodiscard]] auto resourceManager() const -> resources::ResourceGroupManager<Config> const&
    {
        return resourceGroupManager_;
    }

    [[nodiscard]] auto resourceManager() -> resources::ResourceGroupManager<Config>& { return resourceGroupManager_; }
};

template<typename Config>
Root<Config>::Root()
  : RootBase{}
  , resourceGroupManager_{ *this }
{
}
}
#endif // CYCLONITE_ROOT_H
