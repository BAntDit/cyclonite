//
// Created by bantdit on 10/10/20.
//

#include "reader.h"

namespace examples::viewer::gltf {
namespace {
using unique_intance_key_t = std::tuple<size_t, size_t, size_t>;
static void _nodeResourcesCount(
  nlohmann::json const& nodes,
  nlohmann::json const& meshes,
  nlohmann::json const& accessors,
  std::unordered_map<unique_intance_key_t, std::tuple<uint32_t, uint32_t, uint32_t>, cyclonite::hash>& instanceCommands,
  uint32_t& instanceCount,
  uint32_t& vertexCount,
  uint32_t& indexCount,
  size_t nodeIdx)
{
    auto const& node = nodes.at(nodeIdx);

    auto idxMesh = _getOptional(node, reinterpret_cast<char const*>(u8"mesh"), std::numeric_limits<size_t>::max());

    if (idxMesh != std::numeric_limits<size_t>::max()) {
        auto const& mesh = meshes.at(idxMesh);
        auto const& primitives = _getJsonProperty(mesh, reinterpret_cast<char const*>(u8"primitives"));

        for (size_t j = 0, primitiveCount = primitives.size(); j < primitiveCount; j++) {
            auto& primitive = primitives.at(j);

            auto idxIndices =
              _getOptional(primitive, reinterpret_cast<char const*>(u8"indices"), std::numeric_limits<size_t>::max());

            if (idxIndices >= accessors.size()) // skip non-indexed geometry
                continue;

            auto attributes = _getJsonProperty(primitive, reinterpret_cast<char const*>(u8"attributes"));

            auto idxPositions =
              _getOptional(attributes, reinterpret_cast<char const*>(u8"POSITION"), std::numeric_limits<size_t>::max());
            auto idxNormals =
              _getOptional(attributes, reinterpret_cast<char const*>(u8"NORMAL"), std::numeric_limits<size_t>::max());

            if (idxPositions >= accessors.size() || idxNormals >= accessors.size())
                continue;

            auto it = instanceCommands.find(std::make_tuple(idxIndices, idxPositions, idxNormals));
            if (it == instanceCommands.end()) {
                auto vc = _getOptional(
                  accessors.at(idxPositions), reinterpret_cast<char const*>(u8"count"), static_cast<uint32_t>(0));
                auto ic = _getOptional(
                  accessors.at(idxIndices), reinterpret_cast<char const*>(u8"count"), static_cast<uint32_t>(0));

                vertexCount += vc;
                indexCount += ic;

                instanceCommands.emplace(std::make_tuple(idxIndices, idxPositions, idxNormals),
                                         std::make_tuple(0, vc, ic));
            }

            instanceCount++;
        } // primitives cycle end
    }     // mesh parse end

    // children nodes:
    if (auto childrenIt = node.find(reinterpret_cast<char const*>(u8"children")); childrenIt != node.end()) {
        auto const& children = *childrenIt;

        for (size_t i = 0, count = children.size(); i < count; i++) {
            auto idx = children.at(i).get<size_t>();

            _nodeResourcesCount(
              nodes, meshes, accessors, instanceCommands, instanceCount, vertexCount, indexCount, idx);
        }
    }
}
}

auto Reader::resourceCount(std::pair<void const*, size_t> buffer) -> ResourceCount
{
    std::stringstream stream;

    auto [data, size] = buffer;

    stream.write(static_cast<char const*>(data), static_cast<std::streamsize>(size));

    return resourceCount(stream);
}

auto Reader::resourceCount(std::filesystem::path const& path) -> ResourceCount
{
    std::ifstream file;
    file.exceptions(std::ios::failbit);
    file.open(path.string());
    file.exceptions(std::ios::badbit);

    resourceCount(file);
}

auto Reader::resourceCount(std::istream& stream) -> ResourceCount
{
    auto resCount = ResourceCount{};

    auto input = nlohmann::json{};
    stream >> input;

    if (!input.is_object()) {
        throw std::runtime_error("input data is not glTF json");
    }

    auto it = input.find(reinterpret_cast<char const*>(u8"asset"));
    if (it == input.end()) {
        throw std::runtime_error("glTF json must contain asset object");
    }

    nlohmann::json& asset = (*it);
    if (!asset.is_object()) {
        throw std::runtime_error("glTF json must contain asset object");
    }

    if (!_testVersion(asset)) {
        throw std::runtime_error("asset has unsupported glTF version");
    }

    auto const& scenes = _getJsonProperty(input, reinterpret_cast<char const*>(u8"scenes"));
    resCount.sceneCount = static_cast<uint8_t>(scenes.size());

    auto const& buffers = _getJsonProperty(input, reinterpret_cast<char const*>(u8"buffers"));
    resCount.bufferCount = buffers.size();

    {
        using unique_geometry_key_t = std::tuple<size_t, size_t, size_t>;
        std::set<unique_geometry_key_t, std::less<>> uniqueGeometry = {};

        auto const& meshes = _getJsonProperty(input, reinterpret_cast<char const*>(u8"meshes"));
        auto meshCount = meshes.size();

        for (auto meshIndex = size_t{ 0 }; meshIndex < meshCount; meshIndex++) {
            auto const& mesh = meshes.at(meshIndex);
            auto const& primitives = _getJsonProperty(mesh, reinterpret_cast<char const*>(u8"primitives"));

            for (auto j = size_t{ 0 }, primitiveCount = primitives.size(); j < primitiveCount; j++) {
                auto& primitive = primitives.at(j);

                auto idxIndices = _getOptional(
                  primitive, reinterpret_cast<char const*>(u8"indices"), std::numeric_limits<size_t>::max());

                if (idxIndices == std::numeric_limits<size_t>::max()) // skip non-indexed geometry
                    continue;

                auto attributes = _getJsonProperty(primitive, reinterpret_cast<char const*>(u8"attributes"));

                auto idxPositions = _getOptional(
                  attributes, reinterpret_cast<char const*>(u8"POSITION"), std::numeric_limits<size_t>::max());
                auto idxNormals = _getOptional(
                  attributes, reinterpret_cast<char const*>(u8"NORMAL"), std::numeric_limits<size_t>::max());

                if (idxPositions >= std::numeric_limits<size_t>::max() ||
                    idxNormals >= std::numeric_limits<size_t>::max())
                    continue;

                auto geometryIt = uniqueGeometry.find(std::make_tuple(idxIndices, idxPositions, idxNormals));

                if (geometryIt == uniqueGeometry.end()) {
                    uniqueGeometry.emplace(std::make_tuple(idxIndices, idxPositions, idxNormals));
                }
            }
        } // meshes

        resCount.geometryCount = uniqueGeometry.size();
    }

    auto const& animations = _getJsonProperty(input, reinterpret_cast<char const*>(u8"animations"));
    resCount.animationCount = animations.size();

    {
        std::unordered_map<unique_intance_key_t, std::tuple<uint32_t, uint32_t, uint32_t>, cyclonite::hash>
          instanceCommands = {};

        auto instanceCount = uint32_t{ 0 };
        auto vertexCount = uint32_t{ 0 };
        auto indexCount = uint32_t{ 0 };

        auto const& nodes = _getJsonProperty(input, reinterpret_cast<char const*>(u8"nodes"));
        auto defaultSceneIdx = _getOptional(input, reinterpret_cast<char const*>(u8"asset"), static_cast<size_t>(0));
        auto const& meshes = _getJsonProperty(input, reinterpret_cast<char const*>(u8"meshes"));
        auto const& scene = scenes.at(defaultSceneIdx);
        auto const& rootNodes = _getJsonProperty(scene, reinterpret_cast<char const*>(u8"nodes"));
        auto const& accessors = _getJsonProperty(input, reinterpret_cast<char const*>(u8"accessors"));

        for (size_t i = 0, count = rootNodes.size(); i < count; i++) {
            auto idx = rootNodes.at(i).get<size_t>();

            _nodeResourcesCount(
              nodes, meshes, accessors, instanceCommands, instanceCount, vertexCount, indexCount, idx);
        }

        resCount.commandCount = static_cast<uint32_t>(instanceCommands.size());
        resCount.instanceCount = instanceCount;
        resCount.vertexCount = vertexCount;
        resCount.indexCount = indexCount;
    }

    return resCount;
}

void Reader::_countNode(
  nlohmann::json const& nodes,
  nlohmann::json const& meshes,
  nlohmann::json const& accessors,
  std::unordered_map<intance_key_t, std::tuple<uint32_t, uint32_t, uint32_t>, cyclonite::hash>& instanceCommands,
  size_t nodeIdx)
{
    auto const& node = nodes.at(nodeIdx);

    auto idxMesh = _getOptional(node, reinterpret_cast<char const*>(u8"mesh"), std::numeric_limits<size_t>::max());

    if (idxMesh != std::numeric_limits<size_t>::max()) {
        auto const& mesh = meshes.at(idxMesh);
        auto const& primitives = _getJsonProperty(mesh, reinterpret_cast<char const*>(u8"primitives"));

        for (size_t j = 0, primitiveCount = primitives.size(); j < primitiveCount; j++) {
            auto& primitive = primitives.at(j);

            auto idxIndices =
              _getOptional(primitive, reinterpret_cast<char const*>(u8"indices"), std::numeric_limits<size_t>::max());

            if (idxIndices >= accessors.size()) // skip non-indexed geometry
                continue;

            auto attributes = _getJsonProperty(primitive, reinterpret_cast<char const*>(u8"attributes"));

            auto idxPositions =
              _getOptional(attributes, reinterpret_cast<char const*>(u8"POSITION"), std::numeric_limits<size_t>::max());
            auto idxNormals =
              _getOptional(attributes, reinterpret_cast<char const*>(u8"NORMAL"), std::numeric_limits<size_t>::max());

            if (idxPositions >= accessors.size() || idxNormals >= accessors.size())
                continue;

            auto it = instanceCommands.find(std::make_tuple(idxIndices, idxPositions, idxNormals));

            if (it == instanceCommands.end()) {
                auto vc = _getOptional(
                  accessors.at(idxPositions), reinterpret_cast<char const*>(u8"count"), static_cast<uint32_t>(0));
                auto ic = _getOptional(
                  accessors.at(idxIndices), reinterpret_cast<char const*>(u8"count"), static_cast<uint32_t>(0));

                vertexCount_ += vc;
                indexCount_ += ic;

                instanceCommands.emplace(std::make_tuple(idxIndices, idxPositions, idxNormals),
                                         std::make_tuple(0, vc, ic));
            }

            instanceCount_++;
        } // primitives cycle end
    }     // mesh parse end

    if (auto childrenIt = node.find(reinterpret_cast<char const*>(u8"children")); childrenIt != node.end()) {
        auto const& children = *childrenIt;

        for (size_t i = 0, count = children.size(); i < count; i++) {
            auto idx = children.at(i).get<size_t>();
            _countNode(nodes, meshes, accessors, instanceCommands, idx);
        }
    }
}
}
