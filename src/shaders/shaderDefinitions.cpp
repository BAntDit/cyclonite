//
// Created by anton on 4/5/21.
//

#include "shaderDefinitions.h"
#include <cassert>

namespace cyclonite::shaders {
static std::vector<uint32_t> const defaultVertexShaderCode = {
#include "shaders/default.vert.spv.cpp.txt"
};

static std::vector<uint32_t> const defaultFragmentShaderCode = {
#include "shaders/default.frag.spv.cpp.txt"
};

std::vector<uint32_t> const& getShader(ShaderType shaderType)
{
    switch (shaderType) {
        case ShaderType::DEFAULT_VERTEX_TRANSFORM_SHADER:
            return defaultVertexShaderCode;
        case ShaderType::DEFAULT_G_BUFFER_FRAGMENT_SHADER:
            return defaultFragmentShaderCode;
        case ShaderType::SCREEN_TRIANGLE_VERTEX_SHADER:
        case ShaderType::SCREEN_G_BUFFER_DEBUG_FRAGMENT_SHADER:
        default:
            assert(false);
    }
}
}