//
// Created by anton on 2/5/23.
//

#ifndef CYCLONITE_MATERIAL_H
#define CYCLONITE_MATERIAL_H

#include "resources/resource.h"
#include "technique.h"
#include "typedefs.h"
#include <array>
#include <bitset>
#include <boost/functional/hash.hpp>
#include <unordered_map>

namespace cyclonite::compositor {
class NodeIdentifier;
}

namespace cyclonite::vulkan {
class Device;
}

namespace cyclonite::render {
class Technique;

class Material : public resources::Resource
{
public:
public:
    [[nodiscard]] auto instance_tag() const -> ResourceTag const& override { return tag; }

private:
    void addTechnique(vulkan::Device& device,
                      compositor::NodeIdentifier const& nodeIdentifier,
                      size_t passIndex,
                      std::array<resources::Resource::Id, rasterization_shader_stage_count_v> precompiledShaders,
                      std::array<spir_v_code_t const*, rasterization_shader_stage_count_v> spirVCode,
                      std::array<std::string_view, rasterization_shader_stage_count_v> entryPoints,
                      std::bitset<rasterization_shader_stage_count_v> stageMask);

    static ResourceTag tag;

public:
    static auto type_tag_const() -> ResourceTag const& { return Material::tag; }
    static auto type_tag() -> ResourceTag& { return Material::tag; }

private:
    using technique_to_pass_subpass_key_t = std::pair<uint64_t, size_t>; // pass id (node identifier) / sub pass idx
    using technique_hashtable_t =
      std::unordered_map<technique_to_pass_subpass_key_t, Technique, boost::hash<technique_to_pass_subpass_key_t>>;

    std::string name_;
    technique_hashtable_t techniques_;
};
}

#endif // CYCLONITE_MATERIAL_H
