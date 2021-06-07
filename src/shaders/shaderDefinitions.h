//
// Created by anton on 4/4/21.
//

#ifndef CYCLONITE_SHADERDEFINITIONS_H
#define CYCLONITE_SHADERDEFINITIONS_H

#include <cstdint>
#include <vector>

namespace cyclonite::shaders {
enum class ShaderType
{
    DEFAULT_VERTEX_TRANSFORM_SHADER = 0,
    DEFAULT_G_BUFFER_FRAGMENT_SHADER = 1,
    SCREEN_TRIANGLE_VERTEX_SHADER = 2,
    SCREEN_G_BUFFER_DEBUG_FRAGMENT_SHADER = 3,
    MIN_VALUE = DEFAULT_VERTEX_TRANSFORM_SHADER,
    MAX_VALUE = SCREEN_TRIANGLE_VERTEX_SHADER,
    COUNT = MAX_VALUE + 1
};

inline std::vector<uint32_t> const& getShader(ShaderType shaderType);
}

#endif // CYCLONITE_SHADERDEFINITIONS_H
