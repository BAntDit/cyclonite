//
// Created by anton on 9/7/25.
//

#ifndef CYCLONITE_COMMANDPOOLINTERFACE_H
#define CYCLONITE_COMMANDPOOLINTERFACE_H

#include "core/resourceSharedRef.h"
#include "gfx/commandList.h"
#include <concepts>

namespace cyclonite::gfx::interfaces {
template<typename T>
concept CommandPoolConcept = requires(T t, bool a) {
    { t.queueFamilyIndex() } -> std::same_as<uint32_t>;

    { t.allocCommandList() } -> std::same_as<gfx::CommandList>;

    { t.reset(a) } -> std::same_as<void>;

    { t.resourceBase() } -> std::same_as<core::ResourceBase*>;
};

template<CommandPoolConcept PlatformImplementation>
class CommandPoolInterface : private PlatformImplementation
{
public:
    friend class core::ResourceBase;

    using PlatformImplementation::allocCommandList;
    using PlatformImplementation::PlatformImplementation;
    using PlatformImplementation::queueFamilyIndex;
    using PlatformImplementation::reset;
    using PlatformImplementation::resourceBase;
};
}

#endif // CYCLONITE_COMMANDPOOLINTERFACE_H
