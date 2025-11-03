//
// Created by anton on 11/2/25.
//

#ifndef CYCLONITE_VK_SHADER_STAGE_H
#define CYCLONITE_VK_SHADER_STAGE_H

#include "core/resourceWeakRef.h"
#include "gfx/common.h"
#include <array>
#include <memory>
#include <string>

#if defined(GFX_DRIVER_VULKAN)
namespace cyclonite::gfx::vulkan {
class ShaderStage
{
    // the most common use case for specialization constant
    // is ability to specify local work group size in runtime
    // so, 8 specialization entries is mpre then enough
    static constexpr auto max_specialization_entry_count_v = size_t{ 8 };

    struct SpecializationEntry
    {
        uint32_t constantId;
        size_t offset;
        size_t size;
    };

    struct SpecializationInfo
    {
        std::array<SpecializationEntry, max_specialization_entry_count_v> entries;
        uint32_t entryCount;
        size_t dataSize;
        void* data;
    };

public:
    ShaderStage(core::ResourceSharedRef shaderRef,
                ShaderStageCreationFlagBits creationFlags,
                ShaderStageFlags stage,
                std::string_view entryPointName);

    [[nodiscard]] auto shader() const -> core::ResourceWeakRef { return shaderRef_; }

    [[nodiscard]] auto creationFlags() const -> ShaderStageCreationFlagBits { return creationFlags_; }

    [[nodiscard]] auto stage() const -> ShaderStageFlags { return stage_; }

    [[nodiscard]] auto entryPointName() const -> std::string_view { return entryPointName_.data(); }

private:
    core::ResourceWeakRef shaderRef_;
    ShaderStageCreationFlagBits creationFlags_;
    ShaderStageFlags stage_;
    std::string entryPointName_;

    // TODO:: make possible to setup specialization info
    std::unique_ptr<SpecializationInfo> specializationInfo_;
};
}
#endif // GFX_DRIVER_VULKAN
#endif // CYCLONITE_VK_SHADER_STAGE_H