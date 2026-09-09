//
// Created by anton on 8/25/26.
//
#include "assetTool.h"
#include <cassert>
#include <tiny_gltf.h>
#include <string>
#include <iostream>
#include <regex>
#include <bit>

#if defined(_WIN32) // _WIN32 / _WIN64 at once
#ifdef max
#undef max
#endif
#ifdef min
#undef min
#endif
#endif

#include <limits>

namespace cyclonite::tools {
namespace
{
struct TwoIntKey
{
    TwoIntKey() : 
        value{ std::numeric_limits<uint64_t>::max() }
    {}

    explicit TwoIntKey(uint64_t v) : 
        value{ v }
    {}

    TwoIntKey(uint32_t a, uint32_t b) : 
        value{ static_cast<uint64_t>(a) | (static_cast<uint64_t>(b) << 32ull) }
    {}

    uint64_t value;
};

auto semanticFlagToUInt32(shared::VertexFormatFlags flag) -> uint32_t 
{
    auto v = static_cast<uint32_t>(std::countr_zero(metrix::value_cast(flag)));
    assert(metrix::value_cast(flag) == (uint64_t{1} << v));
    return v; 
}

auto attributeNameToSemanticFlag(std::string_view name) -> shared::VertexFormatFlags 
{
    auto r = shared::VertexFormatFlags::UNDEFINED;
    if (name == "POSITION") {
        r = shared::VertexFormatFlags::POSITION;
    } else if (name == "NORMAL") {
        r = shared::VertexFormatFlags::NORMAL;
    } else if (name == "TANGENT") {
        r = shared::VertexFormatFlags::TANGENT;
    } else if (name == "BINORMAL") {
        r = shared::VertexFormatFlags::BINORMAL;
    } else if (name == "JOINTS_0") {
        r = shared::VertexFormatFlags::BONE_INDICES;
    } else if (name == "WEIGHTS_0") {
        r = shared::VertexFormatFlags::BONE_WEIGHTS; 
    } else if (name.starts_with("TEXCOORD_")) {
        auto nameStr = std::string(name);
        auto pattern = std::regex("TEXCOORD _(\\d+)");
        auto match = std::smatch{};

        if (std::regex_search(nameStr, match, pattern)) {
            auto const channelCountPerSet = uint64_t{ 4 }; 
            auto texcoordSet = static_cast<uint64_t>(std::stoi(match[0].str()));

            r = static_cast<shared::VertexFormatFlags>(
              1ull << metrix::value_cast(shared::VertexFormatFlags::TEX_COORD_ZERO_CHANNEL_SHIFT) +
                        texcoordSet * channelCountPerSet);
        }
    }
     
    return r;
}

auto gltfModeToPrimitiveTopology(int const mode) -> shared::AssetPrimitiveTopology 
{
    auto r = shared::AssetPrimitiveTopology::UNDEFINED;

    if (mode == TINYGLTF_MODE_LINE) {
        r = shared::AssetPrimitiveTopology::LINE_LIST;
    } else if (mode == TINYGLTF_MODE_LINE_STRIP) {
        r = shared::AssetPrimitiveTopology::LINE_STRIP;
    } else if (mode == TINYGLTF_MODE_POINTS) {
        r = shared::AssetPrimitiveTopology::POINT_LIST;
    } else if (mode == TINYGLTF_MODE_TRIANGLES) {
        r = shared::AssetPrimitiveTopology::TRIANGLE_LIST;
    } else if (mode == TINYGLTF_MODE_TRIANGLE_STRIP) {
        r = shared::AssetPrimitiveTopology::TRIANGLE_STRIP;
    }
    assert(r != shared::AssetPrimitiveTopology::UNDEFINED);

    return r;
}

void fillAssetAccessor(tinygltf::Model const& model, int gltfAccessorIndex, shared::AssetDataAccessor& assetAccessor)
{
    assert(gltfAccessorIndex != -1);

    assert(gltfAccessorIndex < model.accessors.size());
    auto const& gltfAccessor = model.accessors[gltfAccessorIndex];

    assert(gltfAccessor.bufferView != -1);
    assetAccessor.bufferViewIndex = static_cast<uint32_t>(gltfAccessor.bufferView);

    assert(gltfAccessor.byteOffset > 255);
    assetAccessor.byteOffset = static_cast<uint8_t>(gltfAccessor.byteOffset);

    assetAccessor.elementCount = static_cast<uint32_t>(gltfAccessor.count);

    assetAccessor.type = shared::AssetAccessorDataType::Undefined;
    if (gltfAccessor.type == TINYGLTF_TYPE_SCALAR) {
        assetAccessor.type = shared::AssetAccessorDataType::Scalar;
    } else if (gltfAccessor.type == TINYGLTF_TYPE_VEC2) {
        assetAccessor.type = shared::AssetAccessorDataType::Vector2;
    } else if (gltfAccessor.type == TINYGLTF_TYPE_VEC3) {
        assetAccessor.type = shared::AssetAccessorDataType::Vector3;
    } else if (gltfAccessor.type == TINYGLTF_TYPE_VEC4) {
        assetAccessor.type = shared::AssetAccessorDataType::Vector4;
    } else if (gltfAccessor.type == TINYGLTF_TYPE_MAT2) {
        assetAccessor.type = shared::AssetAccessorDataType::Matrix2x2;
    } else if (gltfAccessor.type == TINYGLTF_TYPE_MAT3) {
        assetAccessor.type = shared::AssetAccessorDataType::Matrix3x3;
    } else if (gltfAccessor.type == TINYGLTF_TYPE_MAT4) {
        assetAccessor.type = shared::AssetAccessorDataType::Matrix4x4;
    }
    assert(assetAccessor.type != shared::AssetAccessorDataType::Undefined);

    assetAccessor.componentType = shared::AssetAccessorComponentType::Undefined;
    if (gltfAccessor.componentType == TINYGLTF_COMPONENT_TYPE_BYTE) {
        assetAccessor.componentType = shared::AssetAccessorComponentType::Int8_t;
    } else if (gltfAccessor.componentType == TINYGLTF_COMPONENT_TYPE_DOUBLE) {
        assetAccessor.componentType = shared::AssetAccessorComponentType::Float64_t;
    } else if (gltfAccessor.componentType == TINYGLTF_COMPONENT_TYPE_FLOAT) {
        assetAccessor.componentType = shared::AssetAccessorComponentType::Float32_t;
    } else if (gltfAccessor.componentType == TINYGLTF_COMPONENT_TYPE_INT) {
        assetAccessor.componentType = shared::AssetAccessorComponentType::Int32_t;
    } else if (gltfAccessor.componentType == TINYGLTF_COMPONENT_TYPE_SHORT) {
        assetAccessor.componentType = shared::AssetAccessorComponentType::Int16_t;
    } else if (gltfAccessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE) {
        assetAccessor.componentType = shared::AssetAccessorComponentType::Uint8_t;
    } else if (gltfAccessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT) {
        assetAccessor.componentType = shared::AssetAccessorComponentType::Uint32_t;
    } else if (gltfAccessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT) {
        assetAccessor.componentType = shared::AssetAccessorComponentType::Uint16_t;
    } 
    assert(assetAccessor.componentType != shared::AssetAccessorComponentType::Undefined);
}
}

/*static */ void AssetTool::doCommand(AssetToolCommand& command)
{
    switch (command.type) {
        case CommandType::GLB_TO_ASSET:
            [[fallthrough]];
        case CommandType::GLTF_TO_ASSET: {
            auto model = tinygltf::Model{};
            auto const& input = std::get<ConversionFromFile>(command.input);
            auto& output = std::get<shared::AssetModuleBinary>(command.output);
            auto loader = tinygltf::TinyGLTF{};
            
            auto err = std::string{};
            auto warn = std::string{};

            if (command.type == CommandType::GLTF_TO_ASSET) {
                if (!loader.LoadASCIIFromFile(&model, &err, &warn, input.path.string())) {
                    throw std::runtime_error{ err };
                }
            } else if (command.type == CommandType::GLB_TO_ASSET) {
                if (!loader.LoadBinaryFromFile(&model, &err, &warn, input.path.string())) {
                    throw std::runtime_error{ err };
                }
            }

            if (!warn.empty()) {
                std::cout << warn << std::endl;
            }

            // gltf -> asset:
            output.buffers.reserve(model.buffers.size());
            for (auto const& gltfBuffer : model.buffers) {
                auto& assetBuffer = output.buffers.emplace_back();
                assetBuffer.reserve(gltfBuffer.data.size());

                for (auto byte : gltfBuffer.data) {
                    assetBuffer.push_back(std::byte{ byte });
                }
            }

            output.bufferViews.reserve(model.bufferViews.size());
            for (auto const& gltfBufferView : model.bufferViews) {
                auto& assetBufferView = output.bufferViews.emplace_back();
                assetBufferView.bufferIndex = static_cast<uint32_t>(gltfBufferView.buffer);
                assetBufferView.offset = static_cast<uint32_t>(gltfBufferView.byteOffset);
                assetBufferView.size = static_cast<uint32_t>(gltfBufferView.byteLength);
                assetBufferView.stride = static_cast<uint32_t>(gltfBufferView.byteStride);
            }

            output.materials.reserve(model.materials.size());
            for (auto const& gltfMaterial : model.materials) {
                auto& assetMaterial = output.materials.emplace_back();
                assetMaterial.name = gltfMaterial.name;
            }

            auto subMeshCount = uint32_t{0};
            // key pair { semantic, accessor }, val: index in atrribute array
            auto attributesMap = std::unordered_map<uint64_t, size_t>{}; 
            auto accessorMap = std::unordered_map<uint32_t, size_t>{};

            for (auto const& gltfMesh : model.meshes) {
                subMeshCount += gltfMesh.primitives.size();

                for (auto const& gltfPrimitive : gltfMesh.primitives) {
                    for (auto const& [attrName, accessorIdx] : gltfPrimitive.attributes) {
                        auto semanticFlag = attributeNameToSemanticFlag(attrName);
                        auto semanticKey = semanticFlagToUInt32(semanticFlag);
                        auto uintAccessor = static_cast<uint32_t>(accessorIdx);
                        auto key = TwoIntKey{ semanticKey, uintAccessor };
                        attributesMap.insert(key.value, std::numeric_limits<size_t>::max());
                        accessorMap.emplace(uintAccessor, std::numeric_limits<size_t>::max());
                    }
                }
            }

            output.subMeshes.reserve(subMeshCount);
            output.subMeshAttributes.reserve(attributesMap.size());
            output.dataAccessors.reserve(accessorMap.size());

            for (auto const& gltfMesh : model.meshes) { // meshes
                auto& assetMesh = output.meshes.emplace_back();
                assetMesh.name = gltfMesh.name;
                assetMesh.subMeshes.reserve(gltfMesh.primitives.size());

                for (auto const& gltfPrimitive : gltfMesh.primitives) { // submesh
                    auto subMeshIndex = static_cast<uint32_t>(output.subMeshes.size());
                    auto& assetSubMesh = output.subMeshes.emplace_back();

                    assetSubMesh.primitiveTopology = gltfModeToPrimitiveTopology(gltfPrimitive.mode);

                    if (gltfPrimitive.indices != -1) {
                        auto accessorIdx = static_cast<uint32_t>(gltfPrimitive.indices);
                        assert(accessorMap.count(accessorIdx) > 0);
                        auto assetAccessorIdx = accessorMap[accessorIdx];
                        if (assetAccessorIdx < output.dataAccessors.size()) {
                            assetSubMesh.indices = static_cast<uint32_t>(assetAccessorIdx);
                        } else {
                            assetAccessorIdx = (accessorMap[accessorIdx] = output.dataAccessors.size());
                            fillAssetAccessor(model, gltfPrimitive.indices, output.dataAccessors.emplace_back());
                            assetSubMesh.indices = static_cast<uint32_t>(assetAccessorIdx);
                        }
                    } else {
                        assetSubMesh.indices = std::numeric_limits<uint32_t>::max();
                    }

                    for (auto const& [attrName, accessorIdx] : gltfPrimitive.attributes) {
                        auto semanticFlag = attributeNameToSemanticFlag(attrName);
                        auto semanticKey = semanticFlagToUInt32(semanticFlag);
                        auto uintAccessor = static_cast<uint32_t>(accessorIdx);
                        auto key = TwoIntKey{ semanticKey, uintAccessor };

                        assert(accessorMap.count(uintAccessor) > 0);
                        auto assetAccessorIdx = accessorMap[uintAccessor];
                        if (assetAccessorIdx > output.dataAccessors.size()) {
                            assetAccessorIdx = (accessorMap[accessorIdx] = output.dataAccessors.size());
                            fillAssetAccessor(model, accessorIdx, output.dataAccessors.emplace_back());
                        }

                        assert(attributesMap.count(key.value) > 0);
                        auto assetAttributeIdx = attributesMap[key.value];
                        if (assetAttributeIdx < output.subMeshAttributes.size()) {
                            assetSubMesh.attributes.push_back(static_cast<uint32_t>(assetAttributeIdx));
                        } else {
                            assetAttributeIdx = (attributesMap[key.value] = output.subMeshAttributes.size());
                            auto& assetAttribute = output.subMeshAttributes.emplace_back();

                            assetAttribute.semantic = semanticFlag;
                            assetAttribute.attributeAccessor = assetAccessorIdx;
                        }
                    }

                    assetSubMesh.material = static_cast<uint32_t>(gltfPrimitive.material);

                    assetMesh.subMeshes.push_back(subMeshIndex);
                }
            }

            // asset serialization:

            /*auto accessor = tinygltf::Accessor{};
            auto mesh = tinygltf::Mesh{};
            auto primitive = tinygltf::Primitive{};

            primitive.attributes*/

            // model.
            // model.buffers
        } break;
        default:
            assert(false);
    }
}
}
