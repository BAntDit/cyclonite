//
// Created by anton on 3/21/26.
//

#ifndef CYCLONITE_SHADER_H
#define CYCLONITE_SHADER_H

#include <boost/uuid/uuid.hpp>

#include "core/resourceBase.h"
#include "resources/managedResource.h"
#include "gfx/common.h"

namespace cyclonite {
class Shader
  : public core::ResourceBase
  , public resources::ManagedResource<cyclonite::Shader>
{
public:
    Shader(core::ResourceManagerBase* resourceManager,
           core::ResourceId resourceId,
           std::string_view name,
           boost::uuids::uuid const& uuid);

    void loadImpl(std::istream& stream);

private:
    struct raw_data_t {
        std::vector<uint32_t> code;
        gfx::ShaderStageFlags stage;
        std::string entryName;
    };

    std::unique_ptr<raw_data_t> rawData_;
    core::ResourceSharedRef hwShader_;
};
}

#endif // CYCLONITE_SHADER_H
