//
// Created by anton on 4/26/26.
//

#ifndef CYCLONITE_MATERIAL_H
#define CYCLONITE_MATERIAL_H

#include "core/hashTable.h"
#include "core/resourceBase.h"
#include "gfx/common.h"
#include "gfx/renderStates.h"
#include "resources/managedResource.h"
#include <boost/unordered/unordered_flat_map.hpp>
#include <boost/uuid/uuid.hpp>

namespace cyclonite {
class Material
  : public core::ResourceBase
  , public resources::ManagedResource<cyclonite::Material>
{
public:
    using shader_set_t = core::StaticHashTable<core::ResourceSharedRef,
                                               metrix::value_cast(gfx::ShaderStageFlags::STAGE_COUNT),
                                               gfx::ShaderStageFlags>;

    struct Technique
    {
        Technique(core::ResourceSharedRef const& deviceRef,
                  core::ResourceSharedRef passRef,
                  shader_set_t const& shaderSet,
                  gfx::PrimitiveTopology primitiveTopology,
                  bool primitiveRestart,
                  gfx::RasterizationState rasterizationState);

        core::ResourceSharedRef pipelineRef;
    };

    Material(core::ResourceManagerBase* resourceManager,
             core::ResourceId resourceId,
             resources::ResourceGroupBase* resourceGroup,
             std::string_view name,
             boost::uuids::uuid const& uuid);

    [[nodiscard]] auto getPassTechnique(uint64_t renderPassId) const -> Technique const&
    {
        return techniques_.at(renderPassId);
    }

    void addPassTechnique(core::ResourceSharedRef const& deviceRef,
                          core::ResourceSharedRef passRef,
                          shader_set_t const& shaderSet,
                          gfx::PrimitiveTopology primitiveTopology,
                          bool primitiveRestart,
                          gfx::RasterizationState rasterizationState);

    using core::ResourceBase::resourceBase;

private:
    // pass id, technique
    boost::unordered_flat_map<uint64_t, Technique> techniques_;
};
}

#endif // CYCLONITE_MATERIAL_H