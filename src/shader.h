//
// Created by anton on 3/21/26.
//

#ifndef CYCLONITE_SHADER_H
#define CYCLONITE_SHADER_H

#include <boost/uuid/uuid.hpp>

#include "core/resourceBase.h"
#include "gfx/binding.h"
#include "gfx/common.h"
#include "resources/managedResource.h"

namespace cyclonite {
class Shader
  : public core::ResourceBase
  , public resources::ManagedResource<cyclonite::Shader>
{
    friend class resources::ManagedResource<cyclonite::Shader>;

public:
    Shader(core::ResourceManagerBase* resourceManager,
           core::ResourceId resourceId,
           resources::ResourceGroupBase* resourceGroup,
           std::string_view name,
           boost::uuids::uuid const& uuid);

    [[nodiscard]] auto bindings() const -> std::vector<gfx::Binding> const& { return bindings_; }

    [[nodiscard]] auto gfxShader() const -> core::ResourceSharedRef { return hwShader_; }

    using core::ResourceBase::resourceBase;

private:
    void loadImpl(std::istream& stream);

    void prepareImpl();

    struct raw_data_t
    {
        std::vector<uint32_t> code;
        gfx::ShaderStageFlags stage;
        std::string entryName;
    };

    std::unique_ptr<raw_data_t> rawData_;
    core::ResourceSharedRef hwShader_;
    std::vector<gfx::Binding> bindings_;
};
}

#endif // CYCLONITE_SHADER_H
