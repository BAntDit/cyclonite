//
// Created by anton on 3/30/26.
//

#include "shader.h"
#include "deserialization.h"
#include "shaderModuleBinary.h"

namespace cyclonite {
Shader::Shader(core::ResourceManagerBase* resourceManager,
               core::ResourceId resourceId,
               std::string_view name,
               boost::uuids::uuid const& uuid)
  : core::ResourceBase{ resourceManager, resourceId, false }
  , resources::ManagedResource<cyclonite::Shader>{ name, uuid }
{
}

void Shader::loadImpl(std::istream& stream)
{
    [[maybe_unused]] auto magicNumber = uint32_t{ 0 };
    stream.read(reinterpret_cast<char*>(&magicNumber), sizeof(uint32_t));
    assert(magicNumber == shared::SHADER_MODULE_MAGIC_NUMBER);

    auto smBlockHeaders = std::vector<shared::ShaderModuleBlockHeader>{};
    shared::readStream(smBlockHeaders, stream);

    for (auto blockHeader : smBlockHeaders) {
        auto [baseOffset, blockOffset, size, id] = blockHeader;
        stream.seekg(baseOffset + blockOffset, std::ios::beg);

        auto moduleInfo = shared::ShaderInfoBlock{};
        auto code = std::vector<uint32_t>{};
        auto reflectionData = shared::ShaderReflectionData{};

        switch (id) {
            case shared::SHADER_MODULE_INFO_BLOCK: {
                shared::readStream(moduleInfo, stream);
            } break;
            case shared::SHADER_MODULE_SPIRV_BLOCK: {
                shared::readStream(code, stream);
            } break;
            case shared::SHADER_MODULE_REFLECTION_BLOCK: {
                shared::readStream(reflectionData, stream);
            } break;
            default:
                assert(false);
        }
    }

    // TODO::
}
}
