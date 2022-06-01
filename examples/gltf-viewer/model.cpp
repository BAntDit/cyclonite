//
// Created by bantdit on 11/8/20.
//

#include "model.h"
#include "appConfig.h"
#include "gltf/reader.h"
#include "resources/buffer.h"

namespace examples::viewer {
using namespace cyclonite;

Model::Model() noexcept
  : workspace_{ nullptr }
{}

void Model::init(cyclonite::Root& root,
                 std::string const& path,
                 std::shared_ptr<cyclonite::compositor::Workspace> const& workspace)
{
    auto& device = root.device();

    workspace_ = workspace;

    auto& node = workspace_->get(node_type_register_t::node_key_t<MainNodeConfig>{});

    node.systems().get<systems::UniformSystem>().init(device, 1);

    // maps
    std::unordered_map<size_t, cyclonite::resources::Resource::Id> gltfBufferIndexToResourceId{};
    std::unordered_map<size_t, enttx::Entity> nodeIdxToEntity{};

    gltf::Reader reader{};
    std::vector<enttx::Entity> pool{};
    std::unordered_map<std::tuple<size_t, size_t, size_t>, uint64_t, hash> geometryIdentifiers_{};

    reader.read(path, [&](auto dataType, auto&&... args) -> void {
        auto&& t = std::forward_as_tuple(args...);

        if constexpr (gltf::reader_data_test<gltf::ReaderDataType::RESOURCE_COUNT, decltype(dataType)>()) {
            auto [bufferCount, geometryCount] = t;

            constexpr auto expectedBufferCount = size_t{ 1 };
            constexpr auto initialBufferMemory = size_t{ 64 * 1024 * 1024 };

            root.declareResources(bufferCount + geometryCount,
                                  cyclonite::resources::resource_reg_info_t<cyclonite::resources::Buffer,
                                                                            expectedBufferCount,
                                                                            initialBufferMemory>{});
        }

        if constexpr (gltf::reader_data_test<gltf::ReaderDataType::BUFFER_STREAM, decltype(dataType)>()) {
            auto&& [bufferIndex, bufferSize, stream] = t;

            auto bufferId = root.resourceManager().template create<cyclonite::resources::Buffer>(bufferSize);
            root.resourceManager().get(bufferId).load(stream);

            gltfBufferIndexToResourceId.insert(std::pair{ static_cast<size_t>(bufferIndex), bufferId });
        }

        // transform system initialization
        if constexpr (gltf::reader_data_test<gltf::ReaderDataType::NODE_COUNT, decltype(dataType)>()) {
            auto [nodeCount] = t;

            node.systems().get<systems::TransformSystem>().init();

            pool = node.entities().create(
              std::vector<enttx::Entity>(nodeCount, enttx::Entity{ std::numeric_limits<uint64_t>::max() }));
        }

        // mesh system initialization
        if constexpr (gltf::reader_data_test<gltf::ReaderDataType::VERTEX_INDEX_INSTANCE_COUNT, decltype(dataType)>()) {
            auto [vertexCount, indexCount, instanceCount, commandCount] = t;

            auto& meshSystem = node.systems().get<systems::MeshSystem>();

            meshSystem.init(device, 1, commandCount, instanceCount, indexCount, vertexCount);
        }

        // node
        if constexpr (gltf::reader_data_test<gltf::ReaderDataType::NODE, decltype(dataType)>()) {
            auto&& [gltfNode, parentIdx, nodeIdx] = t;
            auto&& [translation, scale, orientation] = gltfNode;

            auto& transformSystem = node.systems().get<systems::TransformSystem>();

            assert(!pool.empty());

            auto entity = pool.front();

            pool.erase(pool.cbegin());

            nodeIdxToEntity.emplace(nodeIdx, entity);

            auto parent = static_cast<enttx::Entity>(std::numeric_limits<uint64_t>::max());

            if (auto it = nodeIdxToEntity.find(parentIdx); it != nodeIdxToEntity.end()) {
                parent = (*it).second;
            }

            transformSystem.create(node.entities(), parent, entity, translation, scale, orientation);
        }

        // geometry
        if constexpr (gltf::reader_data_test<gltf::ReaderDataType::GEOMETRY, decltype(dataType)>()) {
            auto&& [primitive] = t;
            auto&& [posIdx, normalIdx, indexIdx] = primitive;

            auto const& posAccessor = reader.accessors()[posIdx];

            auto&& [positionBufferViewIdx, positionOffset, positionComponentType, posNormalized, vertexCount, posType] =
              posAccessor;

            (void)positionComponentType;
            (void)posNormalized;

            auto const& normalAccessor = reader.accessors()[normalIdx];
            auto&& [normalBufferViewIdx, normalOffset, normalComponentType, normalNormalized, normalCount, normalType] =
              normalAccessor;

            (void)normalComponentType;
            (void)normalNormalized;

            auto const& indicesAccessor = reader.accessors()[indexIdx];
            auto&& [indexBufferViewIdx, indexOffset, indexComponentType, indNormalized, indexCount, indType] =
              indicesAccessor;

            (void)indNormalized;
            (void)indType;

            auto& meshSystem = node.systems().get<systems::MeshSystem>();

            auto geometry = meshSystem.createGeometry(vertexCount, indexCount);

            { // vertex reading
                auto const& posBufferView = reader.bufferViews()[positionBufferViewIdx];
                auto&& [posBufferIdx, posByteOffset, posByteLength, posByteStride] = posBufferView;
                (void)posByteLength;

                assert(gltfBufferIndexToResourceId.count(posBufferIdx));
                auto posBufferId = gltfBufferIndexToResourceId[posBufferIdx];
                auto& posBuffer = root.resourceManager().get(posBufferId).template as<cyclonite::resources::Buffer>();

                auto const& norBufferView = reader.bufferViews()[normalBufferViewIdx];
                auto&& [norBufferIdx, norByteOffset, norByteLength, norByteStride] = norBufferView;
                (void)norByteLength;

                assert(gltfBufferIndexToResourceId.count(norBufferIdx));
                auto norBufferId = gltfBufferIndexToResourceId[norBufferIdx];
                auto& norBuffer = root.resourceManager().get(norBufferId).template as<cyclonite::resources::Buffer>();

                auto posStride = posByteStride == 0
                                   ? posType == reinterpret_cast<char const*>(u8"vec4") ? sizeof(vec4) : sizeof(vec3)
                                   : posByteStride;

                auto norStride = norByteStride == 0
                                   ? normalType == reinterpret_cast<char const*>(u8"vec4") ? sizeof(vec4) : sizeof(vec3)
                                   : norByteStride;

                auto vertexIdx = size_t{ 0 };

                assert(vertexCount == normalCount);
                auto posSrc = posBuffer.template view<real>(posByteOffset + positionOffset, vertexCount, posStride);
                auto norSrc = norBuffer.template view<real>(posByteOffset + normalOffset, normalCount, norStride);

                for (auto& vertex : geometry->vertices()) {
                    auto pos =
                      glm::make_vec3(reinterpret_cast<real const*>(std::next(posSrc.begin(), vertexIdx).ptr()));

                    auto nor =
                      glm::make_vec3(reinterpret_cast<real const*>(std::next(norSrc.begin(), vertexIdx).ptr()));

                    vertex.position = pos;
                    vertex.normal = nor;

                    vertexIdx++;
                }
            } // end vertex reading

            { // fixedPartIndex reading
                auto const& idxBufferView = reader.bufferViews()[indexBufferViewIdx];
                auto&& [idxBufferIdx, idxByteOffset, idxByteLength, idxByteStride] = idxBufferView;
                (void)idxByteLength;

                assert(gltfBufferIndexToResourceId.count(idxBufferIdx));
                auto idxBufferId = gltfBufferIndexToResourceId[idxBufferIdx];
                auto& idxBuffer = root.resourceManager().get(idxBufferId).template as<cyclonite::resources::Buffer>();

                switch (indexComponentType) {
                    case 5121: { // unsigned byte
                        auto stride = idxByteStride == 0 ? sizeof(uint8_t) : idxByteStride;
                        auto src = idxBuffer.template view<uint8_t>(idxByteOffset + indexOffset, indexCount, stride);

                        auto indexIdx = size_t{ 0 };
                        for (auto& index : geometry->indices()) {
                            index = static_cast<index_type_t>(*std::next(src.begin(), indexIdx++));
                        }
                    } break;
                    case 5123: { // unsigned short
                        auto stride = idxByteStride == 0 ? sizeof(uint16_t) : idxByteStride;
                        auto src = idxBuffer.template view<uint16_t>(idxByteOffset + indexOffset, indexCount, stride);

                        auto indexIdx = size_t{ 0 };
                        for (auto& index : geometry->indices()) {
                            index = static_cast<index_type_t>(*std::next(src.begin(), indexIdx++));
                        }
                    } break;
                    case 5125: { // unsigned int
                        auto stride = idxByteStride == 0 ? sizeof(uint32_t) : idxByteStride;
                        auto src = idxBuffer.template view<uint32_t>(idxByteOffset + indexOffset, indexCount, stride);

                        auto indexIdx = size_t{ 0 };
                        for (auto& index : geometry->indices()) {
                            index = static_cast<index_type_t>(*std::next(src.begin(), indexIdx++));
                        }
                    } break;
                }
            } // end fixedPartIndex reading

            geometryIdentifiers_.emplace(std::make_tuple(posIdx, normalIdx, indexIdx), geometry->id());

            meshSystem.requestVertexDeviceBufferUpdate();
        }

        // mesh
        if constexpr (gltf::reader_data_test<gltf::ReaderDataType::MESH, decltype(dataType)>()) {
            auto primitiveToKey = [](gltf::Reader::Primitive const& p) -> std::tuple<size_t, size_t, size_t> {
                auto [pos, nor, idx] = p;
                return std::make_tuple(pos, nor, idx);
            };

            auto&& [primitives, nodeIdx] = t;
            auto& meshSystem = node.systems().get<systems::MeshSystem>();

            assert(nodeIdxToEntity.count(nodeIdx) != 0);
            auto entity = nodeIdxToEntity[nodeIdx];

            if (primitives.size() == 1) {
                auto key = primitiveToKey(primitives[0]);

                assert(geometryIdentifiers_.count(key) != 0);
                auto geometry = geometryIdentifiers_[key];

                meshSystem.createMesh(node.entities(), entity, geometry);
            } else {
                std::vector<uint64_t> geometries{};

                geometries.reserve(primitives.size());

                for (auto&& p : primitives) {
                    auto key = primitiveToKey(p);
                    assert(geometryIdentifiers_.count(key) != 0);

                    geometries.push_back(geometryIdentifiers_[key]);
                }

                meshSystem.createMesh(node.entities(), entity, geometries);
            }
        }
    });

    {
        auto cameraEntity = node.entities().create();

        node.cameraEntity() = cameraEntity;

        auto& transformSystem = node.systems().get<systems::TransformSystem>();

        auto& entities = node.entities();

        transformSystem.create(entities,
                               enttx::Entity{ std::numeric_limits<uint64_t>::max() },
                               cameraEntity,
                               vec3{ 0.f, 0.f, 0.f },
                               vec3{ 1.f },
                               quat{ 1.f, 0.f, 0.f, 0.f });

        auto& cameraSystem = node.systems().get<systems::CameraSystem>();

        cameraSystem.init();
        cameraSystem.createCamera(
          entities, cameraEntity, components::Camera::PerspectiveProjection{ 1.f, 45.f, .1f, 100.f });
    }

    // screen systems initialization
    /*{
        auto& node = workspace_->get(node_type_register_t::node_key_t<SurfaceNodeConfig>{});
        node.systems().get<systems::UniformSystem>().init(device, node.getRenderTargetBase().frameBufferCount());
    }*/
}

void Model::setCameraTransform(mat4 const& transform)
{
    auto& node = workspace_->get(node_type_register_t::node_key_t<MainNodeConfig>{});

    auto* transformComponent = node.entities().getComponent<cyclonite::components::Transform>(node.cameraEntity());

    transformComponent->matrix = transform;
    transformComponent->state = cyclonite::components::Transform::State::UPDATE_COMPONENTS;
}
}
