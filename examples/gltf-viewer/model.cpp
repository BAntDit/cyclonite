//
// Created by bantdit on 11/8/20.
//

#include "model.h"
#include "gltf/reader.h"

namespace examples::viewer {
using namespace cyclonite;

Model::Model() noexcept
  : timeSinceLastUpdate_{ .0f }
  , workspace_{ nullptr }
{}

void Model::init(std::shared_ptr<cyclonite::compositor::Workspace> const& workspace)
{
    workspace_ = workspace;

    /* entities_ = &entities;

    gltf::Reader reader{};
    std::vector<enttx::Entity> pool{};
    std::unordered_map<size_t, enttx::Entity> nodeIdxToEntity{};
    std::unordered_map<std::tuple<size_t, size_t, size_t>, uint64_t, hash> geometryIdentifiers_{};

    reader.read(path, [&](auto&&... args) -> void {
        auto&& t = std::forward_as_tuple(args...);

        // scene node count
        if constexpr (std::is_same_v<std::decay_t<decltype(std::get<0>(t))>, uint32_t>) {
            auto [nodeCount] = t;

            systems.get<systems::TransformSystem>().init(nodeCount);

            pool = entities.create(
              std::vector<enttx::Entity>(nodeCount, enttx::Entity{ std::numeric_limits<uint64_t>::max() }));
        }

        // initial resources
        if constexpr (std::is_same_v<std::decay_t<decltype(std::get<0>(t))>,
                                     std::tuple<uint32_t, uint32_t, uint32_t, uint32_t>>) {
            auto&& [initials] = t;
            auto [vertexCount, indexCount, instanceCount, commandCount] = initials;

            auto& renderSystem = systems.get<systems::RenderSystem>();

            auto const& renderPass = renderSystem.renderPass();
            auto swapChainLength = renderPass.getSwapChainLength();

            auto& meshSystem = systems.get<systems::MeshSystem>();

            meshSystem.init(device, swapChainLength, commandCount, instanceCount, indexCount, vertexCount);
        }

        // node parse
        if constexpr (std::is_same_v<std::decay_t<decltype(std::get<0>(t))>, gltf::Reader::Node>) {
            auto&& [node, parentIdx, nodeIdx] = t;
            auto&& [translation, scale, orientation] = node;

            auto& transformSystem = systems.get<systems::TransformSystem>();

            assert(!pool.empty());

            auto entity = pool.front();

            pool.erase(pool.cbegin());

            nodeIdxToEntity.emplace(nodeIdx, entity);

            auto parent = static_cast<enttx::Entity>(std::numeric_limits<uint64_t>::max());

            if (auto it = nodeIdxToEntity.find(parentIdx); it != nodeIdxToEntity.end()) {
                parent = (*it).second;
            }

            transformSystem.create(entities, parent, entity, translation, scale, orientation);
        }

        // geometry parse
        if constexpr (std::is_same_v<std::decay_t<decltype(std::get<0>(t))>, gltf::Reader::Primitive>) {
            auto&& [primitive] = t;
            auto&& [pos, nor, ind] = primitive;

            auto const& posAccessor = reader.accessors()[pos];

            auto&& [positionBufferViewIdx, positionOffset, positionComponentType, posNormalized, vertexCount, posType] =
              posAccessor;

            (void)positionComponentType;
            (void)posNormalized;

            auto const& norAccessor = reader.accessors()[nor];
            auto&& [normalBufferViewIdx, normalOffset, normalComponentType, norNormalized, norCount, norType] =
              norAccessor;

            (void)normalComponentType;
            (void)norNormalized;

            auto const& idxAccessor = reader.accessors()[ind];
            auto&& [indexBufferViewIdx, indexOffset, indexComponentType, indNormalized, indexCount, indType] =
              idxAccessor;

            (void)indNormalized;
            (void)indType;

            auto& meshSystem = systems.get<systems::MeshSystem>();

            auto geometry = meshSystem.createGeometry(vertexCount, indexCount);

            { // vertex reading
                auto const& posBufferView = reader.bufferViews()[positionBufferViewIdx];
                auto&& [posBufferIdx, posByteOffset, posByteLength, posByteStride] = posBufferView;
                auto const& posBuffer = reader.buffers()[posBufferIdx];

                (void)posByteLength;

                auto const& norBufferView = reader.bufferViews()[normalBufferViewIdx];
                auto&& [norBufferIdx, norByteOffset, norByteLength, norByteStride] = norBufferView;
                auto const& norBuffer = reader.buffers()[norBufferIdx];

                (void)norByteLength;

                auto posStride = posByteStride == 0
                                   ? posType == reinterpret_cast<char const*>(u8"vec4") ? sizeof(vec4) : sizeof(vec3)
                                   : posByteStride;

                auto norStride = norByteStride == 0
                                   ? norType == reinterpret_cast<char const*>(u8"vec4") ? sizeof(vec4) : sizeof(vec3)
                                   : norByteStride;

                auto vertexIdx = size_t{ 0 };

                for (auto& vertex : geometry->vertices()) {
                    auto pos = glm::make_vec3(reinterpret_cast<real const*>(posBuffer.data() + posByteOffset +
                                                                            positionOffset + posStride * vertexIdx));

                    auto nor = glm::make_vec3(reinterpret_cast<real const*>(norBuffer.data() + norByteOffset +
                                                                            normalOffset + norStride * vertexIdx));

                    vertex.position = pos;
                    vertex.normal = nor;

                    vertexIdx++;
                }
            } // end vertex reading

            { // index reading
                auto const& idxBufferView = reader.bufferViews()[indexBufferViewIdx];
                auto&& [idxBufferIdx, idxByteOffset, idxByteLength, idxByteStride] = idxBufferView;
                auto& idxBuffer = reader.buffers()[idxBufferIdx];

                switch (indexComponentType) {
                    case 5121: { // unsigned byte
                        auto stride = idxByteStride == 0 ? sizeof(uint8_t) : idxByteStride;
                        auto src =
                          RawDataView<uint8_t>{ idxBuffer.data(), idxByteOffset + indexOffset, indexCount, stride };
                        auto indexIdx = size_t{ 0 };
                        for (auto& index : geometry->indices()) {
                            index = static_cast<index_type_t>(*std::next(src.begin(), indexIdx++));
                        }
                    } break;
                    case 5123: { // unsigned short
                        auto stride = idxByteStride == 0 ? sizeof(uint16_t) : idxByteStride;
                        auto src =
                          RawDataView<uint16_t>{ idxBuffer.data(), idxByteOffset + indexOffset, indexCount, stride };
                        auto indexIdx = size_t{ 0 };
                        for (auto& index : geometry->indices()) {
                            index = static_cast<index_type_t>(*std::next(src.begin(), indexIdx++));
                        }
                    } break;
                    case 5125: { // unsigned int
                        auto stride = idxByteStride == 0 ? sizeof(uint32_t) : idxByteStride;
                        auto src =
                          RawDataView<uint32_t>{ idxBuffer.data(), idxByteOffset + indexOffset, indexCount, stride };
                        auto indexIdx = size_t{ 0 };
                        for (auto& index : geometry->indices()) {
                            index = static_cast<index_type_t>(*std::next(src.begin(), indexIdx++));
                        }
                    } break;
                }
            } // end index reading

            geometryIdentifiers_.emplace(std::make_tuple(pos, nor, ind), geometry->id());

            meshSystem.requestVertexDeviceBufferUpdate();
        }

        // parse mesh
        if constexpr (std::is_same_v<std::decay_t<decltype(std::get<0>(t))>, std::vector<gltf::Reader::Primitive>>) {
            auto primitiveToKey = [](gltf::Reader::Primitive const& p) -> std::tuple<size_t, size_t, size_t> {
                auto [pos, nor, idx] = p;
                return std::make_tuple(pos, nor, idx);
            };

            auto&& [primitives, nodeIdx] = t;

            auto& meshSystem = systems.get<systems::MeshSystem>();

            assert(nodeIdxToEntity.count(nodeIdx) != 0);
            auto entity = nodeIdxToEntity[nodeIdx];

            if (primitives.size() == 1) {
                auto key = primitiveToKey(primitives[0]);

                assert(geometryIdentifiers_.count(key) != 0);
                auto const& geometry = geometryIdentifiers_[key];

                meshSystem.createMesh(entities, entity, geometry);
            } else {
                std::vector<uint64_t> geometries{};

                geometries.reserve(primitives.size());

                for (auto&& p : primitives) {
                    auto key = primitiveToKey(p);
                    assert(geometryIdentifiers_.count(key) != 0);

                    geometries.push_back(geometryIdentifiers_[key]);
                }

                meshSystem.createMesh(entities, entity, geometries);
            }
        }
    });

    camera_ = entities.create();

    auto& transformSystem = systems.get<systems::TransformSystem>();
    transformSystem.create(*entities_,
                           enttx::Entity{ std::numeric_limits<uint64_t>::max() },
                           camera_,
                           vec3{ 0.f, 0.f, 0.f },
                           vec3{ 1.f },
                           quat{ 1.f, 0.f, 0.f, 0.f });

    auto& cameraSystem = systems.get<systems::CameraSystem>();
    cameraSystem.init();
    cameraSystem.createCamera(entities, camera_, components::Camera::PerspectiveProjection{ 1.f, 45.f, .1f, 100.f }); */
}
}
