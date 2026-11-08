//
// Created by anton on 11/8/26.
//

#ifndef CYCLONITE_COMPOSITION_RENDER_COMPOSITION_H
#define CYCLONITE_COMPOSITION_RENDER_COMPOSITION_H

#include "composition/common.h"
#include "core/resourceSharedRef.h"
#include "core/ringBuffer.h"
#include "enttx/entity.h"
#include "gfx/common.h"
#include <boost/unordered/unordered_flat_map.hpp>
#include <vector>

namespace cyclonite::composition {
class RenderComposition
{
    friend class Compositor;

public:
    struct pass_constants_t
    {
        void* data;
        size_t size;
        uint64_t frameIndex;
    };

    RenderComposition();

    [[nodiscard]] auto getPassId(std::string_view name) const -> uint64_t;
    [[nodiscard]] auto getPass(uint64_t passId) const -> core::ResourceSharedRef const&;
    [[nodiscard]] auto getPass(std::string_view name) const -> core::ResourceSharedRef const&
    {
        return getPass(getPassId(name));
    }

    [[nodiscard]] auto getPassCamera(uint64_t passId) const -> enttx::Entity;
    [[nodiscard]] auto getPassCamera(std::string_view passName) const -> enttx::Entity
    {
        return getPassCamera(getPassId(passName));
    }

    [[nodiscard]] auto getTextureId(std::string_view name) const -> uint32_t;
    [[nodiscard]] auto getTexture(uint32_t textureId) const -> core::ResourceSharedRef;
    [[nodiscard]] auto getTexture(std::string_view name) const -> core::ResourceSharedRef
    {
        return getTexture(getTextureId(name));
    }

    [[nodiscard]] auto getPassConstantsId(std::string_view name) const -> uint32_t;
    [[nodiscard]] auto getPassConstants(uint32_t constantsId) const -> pass_constants_t const&;
    [[nodiscard]] auto getPassConstants(std::string_view name) const -> pass_constants_t const&
    {
        return getPassConstants(getPassConstantsId(name));
    }

    template<typename ConstantsData>
    void setConstants(uint32_t constantsId, uint64_t frameIndex, ConstantsData const& constantsData);

    void releaseConstantsMemory(uint64_t lastCompletedFrameIndex);

private:
    static constexpr size_t max_render_target_per_pass_v = 16;
    static constexpr size_t constants_ring_buffer_size_v = 64 * 1024 * 256;

    struct texture_t
    {
        size_t textureIndex;
        gfx::Format format;
        bool external;
    };

    struct rtv_t
    {
        size_t textureIndex;
        uint16_t layer;
        uint16_t mip;
        gfx::TextureType type;
        gfx::Format format;
    };

    struct depth_t
    {
        size_t textureIndex;
        gfx::Format format;
    };

    struct pass_t
    {
        core::ResourceSharedRef passRef;
        enttx::Entity cameraEntity;
        depth_t depth;
        std::array<uint32_t, max_render_target_per_pass_v> inputs;
        std::array<uint32_t, max_render_target_per_pass_v> targets;
        uint32_t constantIndex;
        uint16_t width;
        uint16_t height;
        RenderPassType type;
    };

    boost::unordered_flat_map<std::string, uint64_t> nameToPass_;
    boost::unordered_flat_map<uint64_t, pass_t> passes_;
    boost::unordered_flat_map<std::string, uint32_t> nameToRenderTarget_;
    boost::unordered_flat_map<uint32_t, rtv_t> renderTargets_;
    boost::unordered_flat_map<std::string, uint32_t> nameToInput_;
    boost::unordered_flat_map<uint32_t, texture_t> textureInputs_;
    boost::unordered_flat_map<std::string, uint32_t> nameToConstant_;
    boost::unordered_flat_map<uint32_t, pass_constants_t> constants_;

    std::vector<core::ResourceSharedRef> externalTextures_;
    std::vector<core::ResourceSharedRef> textures_;
    std::vector<std::byte> constantsBuffer_;
    core::ConditionalRingBuffer<std::byte, uint64_t, constants_ring_buffer_size_v, true> constantsRing_;
};

template<typename ConstantsData>
void RenderComposition::setConstants(uint32_t constantsId, uint64_t frameIndex, ConstantsData const& constantsData)
{
    auto it = constants_.find(constantsId);
    if (it == constants_.end()) {
        auto [itNew, success] = constants_.emplace(constantsId, pass_constants_t{});
        assert(success);
        it = itNew;
    }

    auto& [_, passConstants] = *it;
    auto constantBufferView = core::BufferView<ConstantsData>{};
    if (passConstants.data == nullptr || passConstants.frameIndex != frameIndex) {
        constantBufferView = constantsRing_.reserveToWrite<ConstantsData>(frameIndex, 1);
    } else {
        constantBufferView = core::BufferView<ConstantsData>{ reinterpret_cast<ConstantsData*>(passConstants.data),
                                                              constantsData.offset,
                                                              1 };
    }

    passConstants.data = constantBufferView.data();
    passConstants.size = sizeof(ConstantsData);
    passConstants.frameIndex = frameIndex;

    memccpy(passConstants.data, &constantsData, sizeof(ConstantsData));
}
}

#endif // CYCLONITE_COMPOSITION_RENDER_COMPOSITION_H
