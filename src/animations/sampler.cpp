//
// Created by anton on 6/21/22.
//

#include "sampler.h"
#include "resources/buffer.h"
#include "resources/resourceManager.h"

namespace cyclonite::animations {
Sampler::Sampler() noexcept
  : interpolate_{}
  , inputBufferId_{}
  , outputBufferId_{}
  , input_{ nullptr, std::numeric_limits<size_t>::max(), 0 }
  , output_{ nullptr, std::numeric_limits<size_t>::max(), 0 }
  , rawValue_{}
  , interpolationType_{ InterpolationType::STEP }
  , componentCount_{ 0 }
{
}

Sampler::Sampler(resources::ResourceManager& resourceManager,
                 interpolator_func_t interpolator,
                 resources::Resource::Id inBufferId,
                 resources::Resource::Id outBufferId,
                 size_t inOffset,
                 size_t inStride,
                 size_t outOffset,
                 size_t outStride,
                 size_t valueCount,
                 size_t componentCount,
                 InterpolationType interpolationType) noexcept
  : interpolate_{ interpolator }
  , inputBufferId_{ inBufferId }
  , outputBufferId_{ outBufferId }
  , input_{ resourceManager.get(inBufferId)
              .template as<resources::Buffer>()
              .view<real>(inOffset, valueCount, inStride) }
  , output_{ resourceManager.get(outBufferId)
               .template as<resources::Buffer>()
               .view<real>(outOffset, valueCount * componentCount, outStride) }
  , rawValue_{}
  , interpolationType_{ interpolationType }
  , componentCount_{ static_cast<uint8_t>(componentCount) }
{
}

void Sampler::update(real playtime)
{
    assert(input_.count() > 0);

    auto min = *input_.begin();
    auto max = *(input_.begin() + (static_cast<int32_t>(input_.count()) - 1));

    playtime = std::max(playtime, min);
    playtime = std::min(playtime, max);

    auto key1 = real{ 0.f };
    auto key2 = real{ 0.f };
    auto key_index1 = size_t{ 0 };
    auto key_index2 = size_t{ 0 };

    auto it = std::upper_bound(input_.begin(), input_.end(), playtime);

    assert(it != input_.begin());

    key_index2 = (it == input_.end()) ? input_.count() - 1 : std::distance(input_.begin(), it);
    key_index1 = key_index2 - 1;

    key2 = (it == input_.end()) ? max : *it;
    key1 = *std::prev(it);

    assert(!(playtime < key1));

    auto delta = key2 - key1;

    auto alpha = (key2 > key1) ? (playtime - key1) / (key2 - key1) : 1.f;

    real const* src = nullptr;

    // key index steps over stride
    switch (interpolationType_) {
        case InterpolationType::STEP:
        case InterpolationType::LINEAR:
        case InterpolationType::SPHERICAL: {
            src = (output_.begin() + key_index1).ptr();
        } break;
        case InterpolationType::CUBIC:
        case InterpolationType::CATMULL_ROM: {
            auto const layoutElementCount = size_t{ 2 }; // 2 == [vertex, tangent]

            src = (output_.begin() + key_index1 * layoutElementCount).ptr();
        } break;
        default:
            assert(false);
    }

    interpolate_(alpha, delta, componentCount_, src, rawValue_.data());
}
}