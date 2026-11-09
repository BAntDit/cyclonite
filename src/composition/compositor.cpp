//
// Created by anton on 11/8/26.
//

#include "compositor.h"
#include "gfx/device.h"
#include "gfx/texture.h"
#include <format>
#include <stdexcept>

namespace cyclonite::composition {
Compositor::Compositor(core::ResourceSharedRef const& device)
  : device_{ device }
  , passes_{}
  , textureInputs_{}
  , texturesToCreate_{}
  , constantNames_{}
  , inputsToCreate_{}
  , depthToCreate_{}
  , depthToReuse_{}
  , externalTextures_{}
  , textures_{}
{
}

auto Compositor::addPass(std::string_view name, RenderPassType type) -> Compositor&
{
    if (passes_.contains(name.data())) {
        throw std::runtime_error{ std::format("pass {0} already exists", name.data()) };
    }

    auto [it, success] = passes_.emplace(name.data(), pass_t{});
    assert(success);

    auto& [_, pass] = *it;
    pass.type = type;

    return *this;
}

auto Compositor::getPass(std::string_view passName) -> pass_t&
{
    if (!passes_.contains(passName.data())) {
        throw std::runtime_error{ std::format("pass {0} does not exists", passName.data()) };
    }

    return passes_.at(passName.data());
}

auto Compositor::setPassResolution(std::string_view passName, uint16_t width, uint16_t height) -> Compositor&
{
    auto& pass = getPass(passName);

    pass.width = width;
    pass.height = height;

    return *this;
}

auto Compositor::setPassCamera(std::string_view passName, enttx::Entity cameraEntity) -> Compositor&
{
    auto& pass = getPass(passName);
    pass.cameraEntity = cameraEntity;

    return *this;
}

auto Compositor::addExternalTexture(std::string_view textureName,
                                    core::ResourceSharedRef const& textureRef) -> Compositor&
{
    if (textureInputs_.contains(textureName.data())) {
        throw std::runtime_error{ std::format("texture {0} already exists", textureName.data()) };
    }

    auto [it, success] = textureInputs_.emplace(textureName.data(), texture_t{});
    assert(success);

    auto& [_, textureDesc] = *it;

    auto& texture = textureRef.as<gfx::Texture>();

    textureDesc.format = texture.format();
    textureDesc.external = true;

    textureDesc.textureIndex = externalTextures_.size();
    externalTextures_.emplace_back(textureRef);

    return *this;
}

auto Compositor::addPassDepth(std::string_view passName, gfx::Format format) -> Compositor&
{
    depthToCreate_.emplace(passName, format);

    /*auto& pass = getPass(passName);
    auto& device = device_.as<gfx::Device>();

    auto depthTexIndex = textures_.size();
    auto& depthTexRef = textures_.emplace_back();

    auto allocationFlags = gfx::GpuMemoryAllocationFlagBits{};
    allocationFlags.set(gfx::GpuMemoryAllocationFlags::DEDICATED_MEMORY);

    auto textureAllocationFlags = gfx::TextureCreationFlagBits{};

    auto textureUsageFlags = gfx::TextureUsageFlagBits{};
    textureUsageFlags.set(gfx::TextureUsageFlags::DEPTH_STENCIL_ATTACHMENT);

    auto samplerRef = core::ResourceSharedRef{};

    depthTexRef = device.createTexture(allocationFlags,
                                       textureAllocationFlags,
                                       samplerRef,
                                       gfx::TextureType::TEXTURE_2D,
                                       format,
                                       pass.width,
                                       pass.height,
                                       1,
                                       1,
                                       1,
                                       gfx::TextureTiling::OPTIMAL,
                                       textureUsageFlags);

    pass.depth.textureIndex = depthTexIndex;
    pass.depth.format = format;*/

    return *this;
}

auto Compositor::setPassDepth(std::string_view passName, std::string_view srcPassName) -> Compositor&
{
    depthToReuse_.emplace(passName, srcPassName);
    return *this;
}

auto Compositor::addPassColorOutput(std::string_view passName,
                                    std::string_view outputName,
                                    gfx::TextureType textureType,
                                    gfx::Format format,
                                    uint16_t layer,
                                    uint16_t mip) -> Compositor&
{
    auto [_, success] =
      texturesToCreate_.emplace(outputName.data(), std::make_tuple(passName, textureType, format, layer, mip));
    assert(success);

    return *this;
}

auto Compositor::addPassInputTexture(std::string_view passName,
                                     std::string_view srcPassName,
                                     std::string_view passOutputTextureName,
                                     gfx::Format format) -> Compositor&
{
    inputsToCreate_.emplace_back(std::make_tuple(passName, srcPassName, passOutputTextureName, format));
    return *this;
}

auto Compositor::addPassInputTexture(std::string_view passName,
                                     std::string_view externalTextureName,
                                     gfx::Format format) -> Compositor&
{
    inputsToCreate_.emplace_back(std::make_tuple(passName, std::string_view{}, externalTextureName, format));
    return *this;
}

auto Compositor::addPassInputFromDepth(std::string_view passName, std::string_view srcPassName) -> Compositor&
{
    inputsToCreate_.emplace_back(std::make_tuple(passName, srcPassName, std::string_view{}, gfx::Format::UNDEFINED));
    return *this;
}

auto Compositor::addPassConstant(std::string_view passName, std::string_view constantBufferName) -> Compositor&
{
    constantNames_.emplace_back(passName, constantBufferName);
    return *this;
}

auto Compositor::makeComposition() -> RenderComposition
{
    auto& device = device_.as<gfx::Device>();

    auto composition = RenderComposition{};

    for (auto& [name, desc] : passes_) {
        auto pass = RenderComposition::pass_t{};

        pass.type = desc.type;
        pass.cameraEntity = desc.cameraEntity;
        pass.width = desc.width;
        pass.height = desc.height;

        if (depthToCreate_.contains(name)) {
            auto format = depthToCreate_.at(name);
            auto depthTexIndex = textures_.size();
            auto& depthTexRef = textures_.emplace_back();

            auto allocationFlags = gfx::GpuMemoryAllocationFlagBits{};
            allocationFlags.set(gfx::GpuMemoryAllocationFlags::DEDICATED_MEMORY);

            auto textureAllocationFlags = gfx::TextureCreationFlagBits{};

            auto textureUsageFlags = gfx::TextureUsageFlagBits{};
            textureUsageFlags.set(gfx::TextureUsageFlags::DEPTH_STENCIL_ATTACHMENT);

            auto samplerRef = core::ResourceSharedRef{};

            depthTexRef = device.createTexture(allocationFlags,
                                               textureAllocationFlags,
                                               samplerRef,
                                               gfx::TextureType::TEXTURE_2D,
                                               format,
                                               pass.width,
                                               pass.height,
                                               1,
                                               1,
                                               1,
                                               gfx::TextureTiling::OPTIMAL,
                                               textureUsageFlags);

            pass.depth.textureIndex = depthTexIndex;
            pass.depth.format = format;
        }
        // reused depth is filled in second pass over passes
    }

    auto pass = pass_t{};

    return composition;
}
}
