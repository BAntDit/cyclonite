//
// Created by anton on 11/9/25.
//

#include "vkPipelineManager.h"
#include "vkDevice.h"
#include <algorithm>
#include <ranges>
#include <unordered_set>

#include "gfx/shader.h"

#if defined(GFX_DRIVER_VULKAN)
namespace cyclonite::gfx::vulkan {
namespace {
auto packRasterizationParams(CompareOp depthCompareOp,
                             PolygonMode polygonMode,
                             CullMode cullMode,
                             FrontFace frontFace) -> uint32_t
{
    auto packed = uint32_t{ 0 };
    auto* dst = reinterpret_cast<uint8_t*>(&packed);

    static_assert(sizeof(std::underlying_type_t<CompareOp>) == sizeof(uint8_t));
    auto depthCompByte = metrix::value_cast(depthCompareOp);

    static_assert(sizeof(std::underlying_type_t<PolygonMode>) == sizeof(uint8_t));
    auto polygonModeByte = metrix::value_cast(polygonMode);

    static_assert(sizeof(std::underlying_type_t<CullMode>) == sizeof(uint8_t));
    auto cullModeByte = metrix::value_cast(cullMode);

    static_assert(sizeof(std::underlying_type_t<FrontFace>) == sizeof(uint8_t));
    auto frontFaceByte = metrix::value_cast(frontFace);

    std::memcpy(dst + 0, &depthCompByte, sizeof(uint8_t));
    std::memcpy(dst + 1, &polygonModeByte, sizeof(uint8_t));
    std::memcpy(dst + 2, &cullModeByte, sizeof(uint8_t));
    std::memcpy(dst + 3, &frontFaceByte, sizeof(uint8_t));

    return packed;
}

auto packStencilParams(StencilState stencilState) -> uint64_t
{
    auto packed = uint64_t{ 0 };
    auto* dst = reinterpret_cast<uint8_t*>(&packed);

    static_assert(sizeof(std::underlying_type_t<StencilOp>) == sizeof(uint8_t));
    auto failOpByte = metrix::value_cast(stencilState.failOp);
    auto depthFailOpByte = metrix::value_cast(stencilState.depthFailOp);
    auto passOpByte = metrix::value_cast(stencilState.passOp);

    static_assert(sizeof(std::underlying_type_t<CompareOp>) == sizeof(uint8_t));
    auto compareOpByte = metrix::value_cast(stencilState.compareOp);

    assert(stencilState.compareMask < std::numeric_limits<uint8_t>::max());
    auto compareMaskByte = static_cast<uint8_t>(stencilState.compareMask);

    assert(stencilState.writeMask < std::numeric_limits<uint8_t>::max());
    auto writeMaskByte = static_cast<uint8_t>(stencilState.writeMask);

    assert(stencilState.reference < std::numeric_limits<uint8_t>::max());
    auto referenceByte = static_cast<uint8_t>(stencilState.reference);

    std::memcpy(dst + 0, &failOpByte, sizeof(uint8_t));
    std::memcpy(dst + 1, &depthFailOpByte, sizeof(uint8_t));
    std::memcpy(dst + 2, &passOpByte, sizeof(uint8_t));
    std::memcpy(dst + 3, &compareOpByte, sizeof(uint8_t));
    std::memcpy(dst + 4, &compareMaskByte, sizeof(uint8_t));
    std::memcpy(dst + 5, &writeMaskByte, sizeof(uint8_t));
    std::memcpy(dst + 6, &referenceByte, sizeof(uint8_t));

    return packed;
}
}

PipelineManager::PipelineManager(Device* device)
  : device_{ device }
  , pipelineLayouts_{}
  , pipelineLayoutCount_{ 0 }
  , descriptorSetLayouts_{}
  , descriptorSetLayoutCount_{ 0 }
  , descriptorPools_{}
{
}

auto PipelineManager::getOrCreatePipelineBindingSchema(std::span<Binding const> bindings,
                                                       std::span<PushConstantRange const> pushConstantRanges)
  -> core::ResourceSharedRef
{
    auto bindingSchemaRef = core::ResourceSharedRef();

    auto sets = std::unordered_set<uint32_t>{};
    std::transform(bindings.begin(), bindings.end(), std::inserter(sets, sets.end()), [](auto const& b) -> uint32_t {
        return b.set();
    });

    auto descrSets = std::vector<core::ResourceSharedRef>{};
    descrSets.reserve(sets.size());

    for (auto set : sets) {
        auto sb = bindings | std::views::filter([=](auto const& binding) -> uint32_t { return binding.set() == set; });
        descrSets.emplace_back(getOrCreateDescriptorSetLayout(std::vector<Binding>(sb.begin(), sb.end())));
    }

    for (auto i = uint32_t{ 0 }; i < pipelineLayoutCount_; i++) {
        auto& [vs, vr, p] = pipelineLayouts_[i];

        if (vs.size() == descrSets.size() && vr.size() == pushConstantRanges.size()) {
            if (!std::equal(vs.begin(), vs.end(), descrSets.begin(), [](auto a, auto const& b) -> bool {
                    return a == static_cast<uint64_t>(b.id());
                }))
                continue;

            if (!std::equal(vr.begin(), vr.end(), pushConstantRanges.begin(), [](auto const& a, auto const& b) -> bool {
                    auto [size1, offset1, state1] = a;
                    auto [size2, offset2, state2] = b;

                    return size1 == size2 && offset1 == offset2 && state1 == state2;
                }))
                continue;

            auto& [r, tp] = p;

            tp = std::chrono::high_resolution_clock::now();
            bindingSchemaRef = r;

            break;
        }
    }

    if (!bindingSchemaRef.valid()) {
        if (pipelineLayoutCount_ == max_pipeline_set_layout_count_v) {
            const auto countToRelease = uint32_t{ 16 };
            freePipelineLayouts(countToRelease);
        }

        assert(device_ != nullptr);

        auto& [ds, pc, p] = pipelineLayouts_[pipelineLayoutCount_];
        auto& [r, tp] = p;

        ds.clear();
        std::transform(descrSets.begin(), descrSets.end(), std::back_inserter(ds), [](auto const& r) -> uint64_t {
            return static_cast<uint64_t>(r.id());
        });

        pc = std::vector<PushConstantRange>(pushConstantRanges.begin(), pushConstantRanges.end());

        tp = std::chrono::high_resolution_clock::now();
        r = device_->createPipelineBindingSchema(descrSets, pushConstantRanges);

        descriptorSetLayoutCount_++;

        bindingSchemaRef = r;
    }

    assert(bindingSchemaRef.valid());
    return bindingSchemaRef;
}

auto PipelineManager::getOrCreateDescriptorSetLayout(std::span<Binding const> bindings) -> core::ResourceSharedRef
{
    auto descriptorSetLayoutRef = core::ResourceSharedRef();

    for (auto i = uint32_t{ 0 }; i < descriptorSetLayoutCount_; i++) {
        auto& [vb, p] = descriptorSetLayouts_[i];
        if (vb.size() == bindings.size() && std::equal(vb.begin(), vb.end(), bindings.begin())) {
            auto& [r, tp] = p;
            descriptorSetLayoutRef = r;
            tp = std::chrono::high_resolution_clock::now();

            break;
        }
    }

    if (!descriptorSetLayoutRef.valid()) {
        if (descriptorSetLayoutCount_ == max_descriptor_set_layout_count_v) {
            const auto countToRelease = uint32_t{ 16 };
            freeDescriptorSetLayouts(countToRelease);
        }

        assert(device_ != nullptr);

        auto& [vb, p] = descriptorSetLayouts_[descriptorSetLayoutCount_];
        auto& [r, tp] = p;
        vb = std::vector<Binding>(bindings.begin(), bindings.end());
        tp = std::chrono::high_resolution_clock::now();
        r = device_->createDescriptorSetLayout(bindings);

        descriptorSetLayoutCount_++;

        descriptorSetLayoutRef = r;
    }

    assert(descriptorSetLayoutRef.valid());
    return descriptorSetLayoutRef;
}

auto PipelineManager::getOrCreatePrimitiveRasterizationPipeline(PipelineCreationFlagBits creationFlags,
                                                                core::ResourceSharedRef const& bindingSchemaRef,
                                                                PrimitiveTopology primitiveTopology,
                                                                bool primitiveRestartEnable,
                                                                core::ResourceSharedRef const& renderPassRef,
                                                                RasterizationState const& rasterizationState,
                                                                std::span<core::ResourceSharedRef const> shaders)
  -> core::ResourceSharedRef
{
    assert(bindingSchemaRef.valid());
    assert(renderPassRef.valid());

    auto pipelineRef = core::ResourceSharedRef();

    auto packedRasterParams = packRasterizationParams(rasterizationState.depthComparison,
                                                      rasterizationState.polygonMode,
                                                      rasterizationState.cullMode,
                                                      rasterizationState.frontFace);

    auto packedStencilFront = packStencilParams(rasterizationState.frontStencilState);
    auto packedStencilBack = packStencilParams(rasterizationState.backStencilState);

    auto vertexShaderRef = core::ResourceSharedRef();
    auto tessControlShaderRef = core::ResourceSharedRef();
    auto tessEvalShaderRef = core::ResourceSharedRef();
    auto geometryShaderRef = core::ResourceSharedRef();
    auto fragmentShaderRef = core::ResourceSharedRef();

    for (auto& shaderRef : shaders) {
        auto& shader = shaderRef.as<gfx::Shader>();

        switch (shader.stage()) {
            case ShaderStageFlags::VERTEX:
                if (!vertexShaderRef.valid()) {
                    vertexShaderRef = shaderRef;
                } else {
                    throw std::runtime_error("primitive rasterization pipeline must have only one vertex shader");
                }
                break;
            case ShaderStageFlags::TESSELLATION_CONTROL:
                if (!tessControlShaderRef.valid()) {
                    tessControlShaderRef = shaderRef;
                } else {
                    throw std::runtime_error("primitive rasterization pipeline must have only one tess ctrl shader");
                }
                break;
            case ShaderStageFlags::TESSELLATION_EVALUATION:
                if (!tessControlShaderRef.valid()) {
                    tessEvalShaderRef = shaderRef;
                } else {
                    throw std::runtime_error("primitive rasterization pipeline must have only one tess eval shader");
                }
                break;
            case ShaderStageFlags::GEOMETRY:
                if (!geometryShaderRef.valid()) {
                    geometryShaderRef = shaderRef;
                } else {
                    throw std::runtime_error("primitive rasterization pipeline must have only one geometry shader");
                }
                break;
            case ShaderStageFlags::FRAGMENT:
                if (!geometryShaderRef.valid()) {
                    fragmentShaderRef = shaderRef;
                } else {
                    throw std::runtime_error("primitive rasterization pipeline must have only one fragment shader");
                }
                break;
            default:
                throw std::runtime_error("primitive rasterization pipeline met unexpected shader stage");
        }
    }

    if (!vertexShaderRef.valid()) {
        throw std::runtime_error("primitive rasterization pipeline must contains vertex stage shader");
    }

    if (!fragmentShaderRef.valid()) {
        throw std::runtime_error("primitive rasterization pipeline must contains fragment stage shader");
    }

    auto pipelineRefIt = primitiveRasterizationPipelineCache_.find(
      creationFlags.value,
      static_cast<uint64_t>(bindingSchemaRef.id()),
      metrix::value_cast(primitiveTopology),
      primitiveRestartEnable,
      static_cast<uint64_t>(renderPassRef.id()),
      rasterizationState.flags.value,
      packedRasterParams,
      packedStencilFront,
      packedStencilBack,
      static_cast<uint64_t>(vertexShaderRef.id()),
      (tessControlShaderRef.valid() ? static_cast<uint64_t>(tessControlShaderRef.id()) : uint64_t{ 0 }),
      (tessEvalShaderRef.valid() ? static_cast<uint64_t>(tessEvalShaderRef.id()) : uint64_t{ 0 }),
      (geometryShaderRef.valid() ? static_cast<uint64_t>(geometryShaderRef.id()) : uint64_t{ 0 }),
      static_cast<uint64_t>(fragmentShaderRef.id()));

    if (pipelineRefIt != primitiveRasterizationPipelineCache_.end()) {
        auto& [k, v] = *pipelineRefIt;
        auto& [r, tp] = v;

        pipelineRef = r;
        tp = std::chrono::high_resolution_clock::now();
    } else {
        pipelineRef = device_->createPrimitiveRasterizationPipeline(creationFlags,
                                                                    bindingSchemaRef,
                                                                    primitiveTopology,
                                                                    primitiveRestartEnable,
                                                                    renderPassRef,
                                                                    rasterizationState,
                                                                    shaders);

        constexpr auto max_pipeline_to_release_count_v = size_t{ 32 };
        while (primitiveRasterizationPipelineCache_.size() > max_pipeline_to_release_count_v) {
            auto [_, success] = primitiveRasterizationPipelineCache_.add(
              std::make_pair(pipelineRef, std::chrono::high_resolution_clock::now()),
              creationFlags.value,
              static_cast<uint64_t>(bindingSchemaRef.id()),
              metrix::value_cast(primitiveTopology),
              primitiveRestartEnable,
              static_cast<uint64_t>(renderPassRef.id()),
              rasterizationState.flags.value,
              packedRasterParams,
              packedStencilFront,
              packedStencilBack,
              static_cast<uint64_t>(vertexShaderRef.id()),
              (tessControlShaderRef.valid() ? static_cast<uint64_t>(tessControlShaderRef.id()) : uint64_t{ 0 }),
              (tessEvalShaderRef.valid() ? static_cast<uint64_t>(tessEvalShaderRef.id()) : uint64_t{ 0 }),
              (geometryShaderRef.valid() ? static_cast<uint64_t>(geometryShaderRef.id()) : uint64_t{ 0 }),
              static_cast<uint64_t>(fragmentShaderRef.id()));

            if (success) {
                break;
            }

            auto pipelineToReleaseCount = size_t{ 0 };
            auto pipelineToReleaseIndex = size_t{ 0 };
            auto lastUsageTimestamp = std::chrono::high_resolution_clock::now();

            auto pipelinesToRelease = std::array<typename decltype(primitiveRasterizationPipelineCache_)::iterator_t,
                                                 max_pipeline_to_release_count_v>{};

            for (auto it = primitiveRasterizationPipelineCache_.begin();
                 it != primitiveRasterizationPipelineCache_.end();
                 it++) {
                auto& [k, v] = *it;
                auto& [r, tp] = v;

                if (tp <= lastUsageTimestamp) {
                    lastUsageTimestamp = tp;
                    pipelinesToRelease[pipelineToReleaseIndex++ % max_pipeline_to_release_count_v] = it;
                    pipelineToReleaseCount = std::max(++pipelineToReleaseCount, max_pipeline_to_release_count_v);
                }
            }

            if (pipelineToReleaseCount == 0)
                break;

            for (auto i = size_t{ 0 }; i < pipelineToReleaseCount; i++) {
                auto it = pipelinesToRelease[i];
                primitiveRasterizationPipelineCache_.remove(it);
            }
        } // while
    }

    assert(pipelineRef.valid());
    return pipelineRef;
}

void PipelineManager::freeDescriptorSetLayouts(uint32_t count)
{
    std::ranges::sort(descriptorSetLayouts_, [](auto const& a, auto const& b) -> bool {
        auto const& [_1, p1] = a;
        auto const& [_2, p2] = b;
        auto const& [_3, tp1] = p1;
        auto const& [_4, tp2] = p1;

        return tp1 > tp2; // old one first
    });

    for (auto i = count; i != 0; i--) {
        auto& [vb, p] = descriptorSetLayouts_[i];
        auto& [r, tp] = p;

        vb = std::vector<Binding>{};
        r = core::ResourceSharedRef();

        descriptorSetLayoutCount_--;
    }
}

void PipelineManager::freePipelineLayouts(uint32_t count)
{
    std::ranges::sort(pipelineLayouts_, [](auto const& a, auto const& b) -> bool {
        auto const& [_00, _10, p1] = a;
        auto const& [_01, _11, p2] = b;
        auto const& [_2, tp1] = p1;
        auto const& [_3, tp2] = p1;

        return tp1 > tp2; // old one first
    });

    for (auto i = count; i != 0; i--) {
        auto& pipelineLayout = pipelineLayouts_[i];
        auto& [ds, pc, p] = pipelineLayout;
        auto& [r, tp] = p;

        ds = std::vector<uint64_t>{};
        pc = std::vector<PushConstantRange>{};
        r = core::ResourceSharedRef();

        pipelineLayoutCount_--;
    }
}
}
#endif