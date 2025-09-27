//
// Created by anton on 9/27/25.
//

#ifndef CYCLONITE_GFX_SIGNAL_INTERFACE_H
#define CYCLONITE_GFX_SIGNAL_INTERFACE_H

#include "core/resourceBase.h"
#include "gfx/common.h"
#include <concepts>

namespace cyclonite::gfx::interfaces {
template<typename T>
concept SignalConcept = requires(T t, uint64_t a, uint64_t b) {
                            {
                                t.type()
                                } -> std::same_as<SignalType>;

                            {
                                t.value()
                                } -> std::same_as<uint64_t>;

                            {
                                t.signalFromCpu(a)
                                } -> std::same_as<void>;

                            {
                                t.waitOnCpu(a, b)
                                } -> std::same_as<bool>;
                        };

template<SignalConcept PlatformImplementation>
class SignalInterface : private PlatformImplementation
{
public:
    friend class core::ResourceBase;

    using PlatformImplementation::PlatformImplementation;
    using PlatformImplementation::resourceBase;
    using PlatformImplementation::signalFromCpu;
    using PlatformImplementation::type;
    using PlatformImplementation::value;
    using PlatformImplementation::waitOnCpu;
};
}

#endif // CYCLONITE_GFX_SIGNAL_INTERFACE_H