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
concept CommandPoolConcept = requires(T t, bool a)
{
    {
        t.queueFamilyIndex()
    }
    ->std::same_as<uint32_t>;

    {
        t.allocCommandList()
    }
    ->std::same_as<gfx::CommandList>;

    {
        t.reset(a)
    }
    ->std::same_as<void>;
};

template<CommandPoolConcept PlatformImplementation>
class CommandPoolInterface : private PlatformImplementation
{
public:
    using PlatformImplementation::allocCommandList;
    using PlatformImplementation::PlatformImplementation;
    using PlatformImplementation::queueFamilyIndex;
    using PlatformImplementation::reset;
};
}

#endif // CYCLONITE_COMMANDPOOLINTERFACE_H
