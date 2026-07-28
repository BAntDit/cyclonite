//
// Created by anton on 4/26/26.
//

#ifndef CYCLONITE_MATERIAL_H
#define CYCLONITE_MATERIAL_H

#include <boost/uuid/uuid.hpp>

#include "core/hashTable.h"
#include "core/resourceBase.h"
#include "gfx/common.h"
#include "gfx/renderStates.h"
#include "resources/managedResource.h"

namespace cyclonite {
class Material
  : public core::ResourceBase
  , public resources::ManagedResource<cyclonite::Material>
{
    friend class resources::ManagedResource<cyclonite::Material>;

public:
    using shader_set_t = core::StaticHashTable<core::ResourceSharedRef,
                                               metrix::value_cast(gfx::ShaderStageFlags::STAGE_COUNT),
                                               gfx::ShaderStageFlags>;

    Material(core::ResourceManagerBase* resourceManager,
             core::ResourceId resourceId,
             resources::ResourceGroupBase* resourceGroup,
             std::string_view name,
             boost::uuids::uuid const& uuid);

    auto manualSetup(core::ResourceSharedRef passRef,
                     shader_set_t const& shaderSet,
                     gfx::RasterizationState const& rasterizationState,
                     gfx::PrimitiveTopology primitiveTopology = gfx::PrimitiveTopology::TRIANGLE_LIST,
                     bool primitiveRestart = false) -> std::shared_future<void>;

    [[nodiscard]] auto pipeline() const -> core::ResourceSharedRef { return pipeline_; }

    void prepareImpl();

    using core::ResourceBase::resourceBase;

private:
    struct raw_data_t
    {
        shader_set_t shaderSet;
        core::ResourceSharedRef passRef;
        gfx::PrimitiveTopology primitiveTopology;
        bool primitiveRestart;
        gfx::RasterizationState rasterizationState;
    };

    std::unique_ptr<raw_data_t> rawData_;
    core::ResourceSharedRef pipeline_;
};
}

#endif // CYCLONITE_MATERIAL_H