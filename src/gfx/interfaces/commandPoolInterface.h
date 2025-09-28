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
concept CommandPoolConcept = requires(T t) {
                                 {
                                     t.queueFamilyIndex()
                                     } -> std::same_as<uint32_t>;

                                 {
                                     t.allocCommandList()
                                     } -> std::same_as<gfx::CommandList>;
                             };

template<CommandPoolConcept PlatformImplementation>
class CommandPoolInterface : private PlatformImplementation
{
public:
    using PlatformImplementation::allocCommandList;
    using PlatformImplementation::PlatformImplementation;
    using PlatformImplementation::queueFamilyIndex;
};
}

#endif // CYCLONITE_COMMANDPOOLINTERFACE_H
