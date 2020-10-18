//
// Created by bantdit on 10/10/20.
//

#include "reader.h"

namespace examples::gltf {
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
