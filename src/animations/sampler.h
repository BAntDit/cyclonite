//
// Created by anton on 6/20/22.
//

#ifndef CYCLONITE_ANIMATIONS_SAMPLER_H
#define CYCLONITE_ANIMATIONS_SAMPLER_H

#include "buffers/bufferView.h"
#include "resources/resource.h"
#include "typedefs.h"

namespace cyclonite::resources {
class ResourceManager;
}

namespace cyclonite::animations {
class Sampler
{
    using interpolator_func_t = void (*)(real alpha, real const* a, real const* b, real* out);

    template<typename T>
    using make_func_t = T (*)(real const*);

public:
    Sampler() noexcept;

    Sampler(resources::ResourceManager& resourceManager,
            interpolator_func_t interpolator,
            resources::Resource::Id inBufferId,
            resources::Resource::Id outBufferId,
            size_t inOffset,
            size_t inStride,
            size_t outOffset,
            size_t outStride,
            size_t valueCount,
            size_t componentCount,
            InterpolationType interpolationType) noexcept;

    void update(real playtime);

    template<typename ValueType>
    [[nodiscard]] auto value(make_func_t<ValueType> makeFunc) const -> ValueType;

private:
    interpolator_func_t interpolate_;
    resources::Resource::Id inputBufferId_;
    resources::Resource::Id outputBufferId_;

    buffers::BufferView<real> input_;
    buffers::BufferView<real> output_;

    std::array<real, 64> rawValue_;

    InterpolationType interpolationType_;
};

template<typename ValueType>
auto Sampler::value(make_func_t<ValueType> makeFunc) const -> ValueType
{
    return makeFunc(rawValue_.data());
}
}

#endif // CYCLONITE_ANIMATIONS_SAMPLER_H
