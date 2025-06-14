//
// Created by bantdit on 11/8/20.
//

#include "model.h"
#include "appConfig.h"
#include "compositor/nodeAsset.h"
#include "gltf/reader.h"
#include "resources/buffer.h"
#include "resources/geometry.h"

namespace examples::viewer {
using namespace cyclonite;

Model::Model() noexcept
  : workspace_{ nullptr }
{
}

void Model::init(cyclonite::Root& root,
                 std::string const& path,
                 std::shared_ptr<cyclonite::compositor::Workspace> const& workspace)
{
    auto& device = root.device();

    workspace_ = workspace;

    auto&& [initialNodeCount,
            initialInstanceCount,
            initialVertexCount,
            initialIndexCount,
            initialCommandCount,
            geometryCount,
            animationCount,
            bufferCount,
            sceneCount] = gltf::Reader::resourceCount(path);

    // move expected count as non constexpr argument (reg info field)
    constexpr auto expectedStagingCount = uint32_t{ 4 };
    constexpr auto expectedBufferCount = size_t{ 1 };
    constexpr auto expectedGeometryCount = size_t{ 32 };
    constexpr auto expectedAnimationCount = size_t{ 1 };

    constexpr auto initialInterpolationTaskCount = size_t{ 256 };
    constexpr auto initialSamplersCount = size_t{ 1024 };
    constexpr auto initialBufferMemory = size_t{ 64 * 1024 * 1024 };
    constexpr auto initialStagingMemory = size_t{ 64 * 1024 * 1024 };

    root.resourceManager().registerResources(
      cyclonite::resources::resource_reg_info_t<cyclonite::compositor::NodeAsset<main_component_config_t>, 1, 0>{},
      cyclonite::resources::resource_reg_info_t<cyclonite::compositor::NodeAsset<empty_component_config_t>, 1, 0>{},
      cyclonite::resources::
        resource_reg_info_t<cyclonite::resources::Buffer, expectedBufferCount, initialBufferMemory>{},
      cyclonite::resources::resource_reg_info_t<cyclonite::resources::Geometry, expectedGeometryCount, 0>{},
      cyclonite::resources::
        resource_reg_info_t<cyclonite::resources::Staging, expectedStagingCount, initialStagingMemory>{},
      cyclonite::resources::resource_reg_info_t<cyclonite::animations::SamplerArray,
                                                expectedAnimationCount,
                                                initialSamplersCount * sizeof(cyclonite::animations::Sampler)>{},
      cyclonite::resources::resource_reg_info_t<cyclonite::animations::AnimationInterpolationTaskArray,
                                                expectedAnimationCount,
                                                initialInterpolationTaskCount *
                                                  sizeof(cyclonite::animations::AnimationInterpolationTaskArray)>{},
      cyclonite::resources::resource_reg_info_t<cyclonite::animations::Animation, expectedAnimationCount, 0>{});

    // init systems::
    auto&& animationNode = workspace_->get("animation-node").as(node_type_register_t::node_key_t<MainNodeConfig>{});

    animationNode.systems().template get<cyclonite::systems::AnimationSystem>().init(root.resourceManager(),
                                                                                     root.taskManager());
    animationNode.systems().template get<cyclonite::systems::TransformSystem>().init();

    auto&& gBufferNode = workspace_->get("g-buffer-node").as(node_type_register_t::node_key_t<GBufferNodeConfig>{});

    auto frameBufferCount = gBufferNode.getRenderTargetBase().frameBufferCount();

    gBufferNode.systems().template get<cyclonite::systems::CameraSystem>().init();

    gBufferNode.systems().template get<cyclonite::systems::MeshSystem>().init(
      root, frameBufferCount, initialCommandCount, initialInstanceCount, initialIndexCount, initialVertexCount);

    gBufferNode.systems().template get<cyclonite::systems::UniformSystem>().init(
      root.taskManager(), root.resourceManager(), device, frameBufferCount);

    auto&& surfaceNode = workspace_->get("surface-node").as(node_type_register_t::node_key_t<SurfaceNodeConfig>{});

    // scene
    auto mainSceneId =
      root.resourceManager().template create<cyclonite::compositor::NodeAsset<main_component_config_t>>();

    auto emptySceneId =
      root.resourceManager().template create<cyclonite::compositor::NodeAsset<empty_component_config_t>>();

    animationNode.asset() = mainSceneId;
    gBufferNode.asset() = mainSceneId;
    surfaceNode.asset() = emptySceneId;

    auto pool = std::vector<enttx::Entity>{};
    {
        auto& asset = root.resourceManager()
                        .get(mainSceneId)
                        .template as<cyclonite::compositor::NodeAsset<main_component_config_t>>();

        pool = asset.entities().create(
          std::vector<enttx::Entity>(initialNodeCount, enttx::Entity{ std::numeric_limits<uint64_t>::max() }));
    }

    auto reader = gltf::Reader{};

    auto nodeIdxToEntity = std::unordered_map<size_t, enttx::Entity>{};
    auto gltfBufferIndexToResourceId = std::unordered_map<size_t, cyclonite::resources::Resource::Id>{};
    auto geometryIdentifiers_ = std::unordered_map<std::tuple<size_t, size_t, size_t>, uint64_t, hash>{};
    auto indexToAnimationId = std::unordered_map<size_t, resources::Resource::Id>{};

    reader.read(path, [&](auto dataType, auto&&... args) -> void {
        auto&& t = std::forward_as_tuple(args...);

        if constexpr (gltf::reader_data_test<gltf::ReaderDataType::BUFFER_STREAM, decltype(dataType)>()) {
            auto&& [bufferIndex, bufferSize, stream] = t;

            auto bufferId = root.resourceManager().template create<cyclonite::resources::Buffer>(bufferSize);
            root.resourceManager().get(bufferId).load(stream);

            gltfBufferIndexToResourceId.insert(std::pair{ static_cast<size_t>(bufferIndex), bufferId });
        }

        if constexpr (gltf::reader_data_test<gltf::ReaderDataType::NODE, decltype(dataType)>()) {
            auto&& [gltfNode, parentIdx, nodeIdx] = t;
            auto&& [translation, scale, orientation] = gltfNode;
            auto& transformSystem = animationNode.systems().template get<systems::TransformSystem>();

            assert(!pool.empty());

            auto entity = pool.front();
            pool.erase(pool.cbegin());

            nodeIdxToEntity.emplace(nodeIdx, entity);

            auto parent = static_cast<enttx::Entity>(std::numeric_limits<uint64_t>::max());
            if (auto it = nodeIdxToEntity.find(parentIdx); it != nodeIdxToEntity.end()) {
                parent = (*it).second;
            }

            auto& asset = root.resourceManager()
                            .get(mainSceneId)
                            .template as<cyclonite::compositor::NodeAsset<main_component_config_t>>();

            transformSystem.create(asset.entities(), parent, entity, translation, scale, orientation);
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

            auto& meshSystem = gBufferNode.systems().get<systems::MeshSystem>();

            auto geometryId = meshSystem.createGeometry(vertexCount, indexCount);
            auto& geometry =
              root.resourceManager().get(resources::Resource::Id{ geometryId }).template as<resources::Geometry>();

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

                for (auto& vertex : geometry.vertices()) {
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

                        auto idxIdx = size_t{ 0 };
                        for (auto& index : geometry.indices()) {
                            index = static_cast<index_type_t>(*std::next(src.begin(), idxIdx++));
                        }
                    } break;
                    case 5123: { // unsigned short
                        auto stride = idxByteStride == 0 ? sizeof(uint16_t) : idxByteStride;
                        auto src = idxBuffer.template view<uint16_t>(idxByteOffset + indexOffset, indexCount, stride);

                        auto idxIdx = size_t{ 0 };
                        for (auto& index : geometry.indices()) {
                            index = static_cast<index_type_t>(*std::next(src.begin(), idxIdx++));
                        }
                    } break;
                    case 5125: { // unsigned int
                        auto stride = idxByteStride == 0 ? sizeof(uint32_t) : idxByteStride;
                        auto src = idxBuffer.template view<uint32_t>(idxByteOffset + indexOffset, indexCount, stride);

                        auto idxIdx = size_t{ 0 };
                        for (auto& index : geometry.indices()) {
                            index = static_cast<index_type_t>(*std::next(src.begin(), idxIdx++));
                        }
                    } break;
                    default:
                        assert(false);
                }
            } // end fixedPartIndex reading

            geometryIdentifiers_.emplace(std::make_tuple(posIdx, normalIdx, indexIdx), geometry.id());

            meshSystem.requestVertexDeviceBufferUpdate();
        }

        // mesh
        if constexpr (gltf::reader_data_test<gltf::ReaderDataType::MESH, decltype(dataType)>()) {
            auto primitiveToKey = [](gltf::Reader::Primitive const& p) -> std::tuple<size_t, size_t, size_t> {
                auto [pos, nor, idx] = p;
                return std::make_tuple(pos, nor, idx);
            };

            auto&& [primitives, nodeIdx] = t;

            assert(nodeIdxToEntity.count(nodeIdx) != 0);
            auto entity = nodeIdxToEntity[nodeIdx];

            auto& meshSystem = gBufferNode.systems().template get<systems::MeshSystem>();

            auto& asset = root.resourceManager()
                            .get(mainSceneId)
                            .template as<cyclonite::compositor::NodeAsset<main_component_config_t>>();

            if (primitives.size() == 1) {
                auto key = primitiveToKey(primitives[0]);

                assert(geometryIdentifiers_.count(key) != 0);
                auto geometry = geometryIdentifiers_[key];

                meshSystem.createMesh(asset.entities(), entity, geometry);
            } else {
                std::vector<uint64_t> geometries{};

                geometries.reserve(primitives.size());

                for (auto&& p : primitives) {
                    auto key = primitiveToKey(p);
                    assert(geometryIdentifiers_.count(key) != 0);

                    geometries.push_back(geometryIdentifiers_[key]);
                }

                meshSystem.createMesh(asset.entities(), entity, geometries);
            }
        }

        if constexpr (gltf::reader_data_test<gltf::ReaderDataType::ANIMATION, decltype(dataType)>()) {
            auto&& [sampleCount, duration, animationIndex] = t;
            auto animationId = root.resourceManager().template create<cyclonite::animations::Animation>(
              root.taskManager(), sampleCount, duration);

            indexToAnimationId.insert(std::pair{ animationIndex, animationId });
        }

        if constexpr (gltf::reader_data_test<gltf::ReaderDataType::ANIMATION_SAMPLER, decltype(dataType)>()) {
            auto [animationIndex,
                  samplerIndex,
                  inputBufferIndex,
                  inputOffset,
                  inputStride,
                  outputBufferIndex,
                  outputOffset,
                  outputStride,
                  componentCount,
                  elementCount,
                  interpolationType,
                  interpolationElementType] = t;

            assert(gltfBufferIndexToResourceId.count(inputBufferIndex));
            auto inputBufferId = gltfBufferIndexToResourceId.at(inputBufferIndex);

            assert(gltfBufferIndexToResourceId.count(outputBufferIndex));
            auto outputBufferId = gltfBufferIndexToResourceId.at(outputBufferIndex);

            assert(indexToAnimationId.contains(animationIndex));
            auto animationId = indexToAnimationId.at(animationIndex);
            auto& animation = root.resourceManager().get(animationId).template as<cyclonite::animations::Animation>();

            animation.setupSampler(samplerIndex,
                                   inputBufferId,
                                   outputBufferId,
                                   inputOffset,
                                   inputStride,
                                   outputOffset,
                                   outputStride,
                                   elementCount,
                                   componentCount,
                                   interpolationType,
                                   interpolationElementType);
        }

        if constexpr (gltf::reader_data_test<gltf::ReaderDataType::ANIMATOR, decltype(dataType)>()) {
            auto&& [nodeIdx, channelCount] = t;

            assert(nodeIdxToEntity.count(nodeIdx) != 0);
            auto entity = nodeIdxToEntity[nodeIdx];

            auto& asset = root.resourceManager()
                            .get(mainSceneId)
                            .template as<cyclonite::compositor::NodeAsset<main_component_config_t>>();

            auto& animator = asset.entities().template assign<components::Animator>(entity, channelCount);
            (void)animator;
        }

        // channel
        if constexpr (gltf::reader_data_test<gltf::ReaderDataType::ANIMATION_CHANNEL, decltype(dataType)>()) {
            auto&& [nodeIdx, animationIndex, idxSampler, channelIndex, animationTarget] = t;

            assert(nodeIdxToEntity.count(nodeIdx) != 0);
            auto entity = nodeIdxToEntity[nodeIdx];

            auto& asset = root.resourceManager()
                            .get(mainSceneId)
                            .template as<cyclonite::compositor::NodeAsset<main_component_config_t>>();

            assert(asset.entities().template hasComponent<components::Animator>(entity));

            auto* animator = asset.entities().template getComponent<components::Animator>(entity);
            auto& channel = animator->getChannel(channelIndex);

            assert(indexToAnimationId.contains(animationIndex));
            auto animationId = indexToAnimationId.at(animationIndex);

            channel.animationId = static_cast<uint64_t>(animationId);
            channel.samplerIndex = idxSampler;

            if (animationTarget == gltf::AnimationTarget::TRANSLATION) {
                channel.update_func = [](void* context, enttx::Entity entity, real const* value) -> void {
                    using entity_manager_t = typename enttx::EntityManager<MainNodeConfig::component_config_t>;
                    auto* entities = reinterpret_cast<entity_manager_t*>(context);
                    auto* transform = entities->template getComponent<components::Transform>(entity);

                    transform->position = glm::make_vec3(value);
                    transform->state = components::Transform::State::UPDATE_LOCAL;
                };
            } else if (animationTarget == gltf::AnimationTarget::SCALE) {
                channel.update_func = [](void* context, enttx::Entity entity, real const* value) -> void {
                    using entity_manager_t = typename enttx::EntityManager<MainNodeConfig::component_config_t>;
                    auto* entities = reinterpret_cast<entity_manager_t*>(context);
                    auto* transform = entities->template getComponent<components::Transform>(entity);

                    transform->scale = glm::make_vec3(value);
                    transform->state = components::Transform::State::UPDATE_LOCAL;
                };
            } else if (animationTarget == gltf::AnimationTarget::ROTATION) {
                channel.update_func = [](void* context, enttx::Entity entity, real const* value) -> void {
                    using entity_manager_t = typename enttx::EntityManager<MainNodeConfig::component_config_t>;
                    auto* entities = reinterpret_cast<entity_manager_t*>(context);
                    auto* transform = entities->template getComponent<components::Transform>(entity);

                    transform->orientation = glm::make_quat(value);
                    transform->state = components::Transform::State::UPDATE_LOCAL;
                };
            } else if (animationTarget == gltf::AnimationTarget::WEIGHTS) {
                throw std::runtime_error("not implemented yet");
            }
        }
    });

    // camera
    {
        auto& asset = root.resourceManager()
                        .get(mainSceneId)
                        .template as<cyclonite::compositor::NodeAsset<main_component_config_t>>();

        auto cameraEntity = asset.entities().create();

        auto& transformSystem = animationNode.systems().template get<cyclonite::systems::TransformSystem>();
        auto& cameraSystem = gBufferNode.systems().template get<cyclonite::systems::CameraSystem>();

        transformSystem.create(asset.entities(),
                               enttx::Entity{ std::numeric_limits<uint64_t>::max() },
                               cameraEntity,
                               vec3{ 0.f, 0.f, 0.f },
                               vec3{ 1.f },
                               quat{ 1.f, 0.f, 0.f, 0.f });

        cameraSystem.createCamera(
          asset.entities(), cameraEntity, components::Camera::PerspectiveProjection{ 1.f, 45.f, .1f, 100.f });

        cameraSystem.renderCamera() = cameraEntity;
    }

    {
        auto animationId = indexToAnimationId.at(0);
        auto& animation = root.resourceManager().get(animationId).template as<cyclonite::animations::Animation>();

        animation.loop(true);
        animation.play();
    }
}

void Model::setCameraTransform(mat4 const& transform)
{
    auto&& animationNode = workspace_->get("animation-node").as(node_type_register_t::node_key_t<MainNodeConfig>{});
    auto&& gBufferNode = workspace_->get("g-buffer-node").as(node_type_register_t::node_key_t<GBufferNodeConfig>{});
    auto& cameraSystem = gBufferNode.systems().template get<cyclonite::systems::CameraSystem>();

    auto* transformComponent =
      animationNode.entities().getComponent<cyclonite::components::Transform>(cameraSystem.renderCamera());

    transformComponent->matrix = transform;
    transformComponent->state = cyclonite::components::Transform::State::UPDATE_COMPONENTS;
}

void Model::dispose()
{
    workspace_.reset();
}
}
