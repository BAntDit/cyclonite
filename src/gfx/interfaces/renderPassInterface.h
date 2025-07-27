//
// Created by anton on 7/26/25.
//

#ifndef CYCLONITE_RENDERPASSINTERFACE_H
#define CYCLONITE_RENDERPASSINTERFACE_H

namespace cyclonite::gfx::interfaces {
template<typename T>
concept RenderPassConcept = true;

template<RenderPassConcept PlatformImplementation>
class RenderPassInterface : private PlatformImplementation
{
public:
    using PlatformImplementation::PlatformImplementation;
};
}

#endif // CYCLONITE_RENDERPASSINTERFACE_H
