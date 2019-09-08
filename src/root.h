//
// Created by bantdit on 9/8/19.
//

#ifndef CYCLONITE_ROOT_H
#define CYCLONITE_ROOT_H

#include "config.h"
#include "options.h"
#include "vulkan/instance.h"
#include <memory>

namespace cyclonite {
template<typename Config>
class Root;

template<typename ComponentList, typename ComponentStorageList, typename SystemList, size_t updateStageCount>
class Root<Config<ComponentList, ComponentStorageList, SystemList, updateStageCount>>
{
public:
    using config_t = Config<ComponentList, ComponentStorageList, SystemList, updateStageCount>;

    Root();

    Root(Root const&) = delete;

    Root(Root&&) = delete;

    ~Root() = default;

    auto operator=(Root const&) -> Root& = delete;

    auto operator=(Root &&) -> Root& = delete;

    void init(Options const& options);

private:
    std::unique_ptr<vulkan::Instance> vulkanInstance_;
};

template<typename ComponentList, typename ComponentStorageList, typename SystemList, size_t updateStageCount>
Root<Config<ComponentList, ComponentStorageList, SystemList, updateStageCount>>::Root()
  : vulkanInstance_{ nullptr }
{}

template<typename ComponentList, typename ComponentStorageList, typename SystemList, size_t updateStageCount>
void Root<Config<ComponentList, ComponentStorageList, SystemList, updateStageCount>>::init(Options const& options)
{
    // todo:: ...
}
}
#endif // CYCLONITE_ROOT_H
