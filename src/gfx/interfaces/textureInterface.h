
#ifndef GFX_INTERFACES_TEXTURE_H
#define GFX_INTERFACES_TEXTURE_H
#include "core/resourceRef.h"
#include "gfx/common.h"
#include <concepts>
#include <cstdint>

namespace cyclonite::gfx::interfaces {
template<typename T>
concept TextureConcept = requires(T t) {
                             {
                                 t.currentState()
                                 } -> std::same_as<TextureState>;

                             {
                                 t.format()
                                 } -> std::same_as<Format>;

                             {
                                 t.width()
                                 } -> std::same_as<uint32_t>;

                             {
                                 t.height()
                                 } -> std::same_as<uint32_t>;

                             {
                                 t.depth()
                                 } -> std::same_as<uint32_t>;

                             {
                                 t.mipCount()
                                 } -> std::same_as<uint32_t>;
                         };

template<TextureConcept PlatformImplementation>
class TextureInterface : private PlatformImplementation
{
public:
    friend class core::ResourceBase;

    using PlatformImplementation::currentState;
    using PlatformImplementation::depth;
    using PlatformImplementation::format;
    using PlatformImplementation::height;
    using PlatformImplementation::mipCount;
    using PlatformImplementation::PlatformImplementation;
    using PlatformImplementation::width;
};
}

#endif // GFX_INTERFACES_TEXTURE_H