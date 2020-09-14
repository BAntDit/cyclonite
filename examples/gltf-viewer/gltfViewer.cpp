//
// Created by bantdit on 8/2/20.
//

#include "gltfViewer.h"
#include "gltf/reader.h"

using namespace cyclonite;
using namespace easy_mp;

namespace examples {
GLTFViewer::GLTFViewer()
  : root_{ std::make_unique<cyclonite::Root>() }
  , entities_{}
  , systems_{ &entities_ }
{}

auto GLTFViewer::init(cyclonite::Options const& options) -> GLTFViewer&
{
    root_->init(options);

    {
        auto& renderSystem = systems_.get<systems::RenderSystem>();

        Options::WindowProperties windowProperties{};

        windowProperties.title = "gltf-viewer.example";
        windowProperties.fullscreen = false;
        windowProperties.left = 0;
        windowProperties.top = 0;
        windowProperties.width = 512;
        windowProperties.height = 512;

        renderSystem.init(
          root_->taskManager(),
          root_->device(),
          windowProperties,
          render_target_output<type_list<render_target_output_candidate<VK_FORMAT_D32_SFLOAT>>>{},
          render_target_output<
            type_list<render_target_output_candidate<VK_FORMAT_B8G8R8A8_UNORM, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR>>,
            RenderTargetOutputSemantic::DEFAULT>{},
          VkClearDepthStencilValue{ 1.0f, 0 },
          VkClearColorValue{ { 0.0f, 0.0f, 0.0f, 1.0f } },
          std::array{ VK_PRESENT_MODE_MAILBOX_KHR, VK_PRESENT_MODE_FIFO_KHR });
    }

    {
        auto& cameraSystem = systems_.get<systems::CameraSystem>();
        cameraSystem.init();
    }

    {
        auto& uniformSystem = systems_.get<systems::UniformSystem>();
        uniformSystem.init(root_->device());
    }

    {
        gltf::Reader reader{};
        std::vector<enttx::Entity> entities{};
        std::unordered_map<size_t, enttx::Entity> parents{};
        std::unordered_map<std::tuple<size_t, size_t, size_t>, uint64_t, hash> geometryIdentifiers_{};

        reader.read("./assets/models/model.gltf", [&](auto&&... args) -> void {
            auto&& t = std::forward_as_tuple(args...);

            if constexpr (std::is_same_v<std::decay_t<decltype(std::get<0>(t))>, uint32_t>) {
                auto [nodeCount] = t;
                auto& transformSystem = systems_.get<systems::TransformSystem>();

                transformSystem.init(nodeCount);

                entities.resize(nodeCount);

                entities_.create(entities);
            }

            if constexpr (std::is_same_v<std::decay_t<decltype(std::get<0>(t))>,
                                         std::tuple<uint32_t, uint32_t, uint32_t, uint32_t>>) {
                auto&& [initials] = t;
                auto [vertexCount, indexCount, instanceCount, commandCount] = initials;

                auto& meshSystem = systems_.get<systems::MeshSystem>();
                meshSystem.init(root_->device(), commandCount, instanceCount, indexCount, vertexCount);
            }

            if constexpr (std::is_same_v<std::decay_t<decltype(std::get<0>(t))>, gltf::Reader::Node>) {
                auto&& [node, parentIdx, nodeIdx] = t;
                auto&& [translation, scale, orientation] = node;

                auto& transformSystem = systems_.get<systems::TransformSystem>();

                assert(!entities.empty());

                auto entity = entities.back();

                entities.pop_back();

                parents.insert(nodeIdx, entity);

                auto parent = static_cast<enttx::Entity>(std::numeric_limits<uint64_t>::max());

                if (auto it = parents.find(parentIdx); it != parents.end()) {
                    parent = (*it).second;
                }

                transformSystem.create(entities_, parent, entity, translation, scale, orientation);
            }

            if constexpr (std::is_same_v<std::decay_t<decltype(std::get<0>(t))>, gltf::Reader::Primitive>) {
                auto&& [primitive] = t;
                auto&& [pos, nor, ind] = primitive;

                auto const& posAccessor = reader.accessors()[pos];
                auto&& [positionBufferViewIdx,
                        positionOffset,
                        positionComponentType,
                        posNormalized,
                        vertexCount,
                        posType] = posAccessor;

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

                auto& meshSystem = systems_.get<systems::MeshSystem>();

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

                    auto posStride =
                      posByteStride == 0
                        ? posType == reinterpret_cast<char const*>(u8"vec4") ? sizeof(vec4) : sizeof(vec3)
                        : posByteStride;

                    auto norStride =
                      norByteStride == 0
                        ? norType == posType == reinterpret_cast<char const*>(u8"vec4") ? sizeof(vec4) : sizeof(vec3)
                        : norByteStride;

                    auto vertexIdx = size_t{ 0 };

                    for (auto& vertex : geometry.vertices()) {
                        auto pos = glm::make_vec3(reinterpret_cast<real const*>(
                          posBuffer.data() + posByteOffset + positionOffset + posStride * vertexIdx));

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
                    auto const& idxBuffer = reader.buffers()[idxBufferIdx];

                    switch (indexComponentType) {
                        case 5121: { // unsigned byte
                            auto stride = idxByteStride == 0 ? sizeof(uint8_t) : idxByteStride;
                            auto src =
                              RawDataView<uint8_t>{ idxBuffer.data(), idxByteOffset + indexOffset, indexCount, stride };
                            auto indexIdx = size_t{ 0 };
                            for (auto& index : geometry.indices()) {
                                index = static_cast<index_type_t>(*std::next(src.begin(), indexIdx++));
                            }
                        } break;
                        case 5123: { // unsigned short
                            auto stride = idxByteStride == 0 ? sizeof(uint16_t) : idxByteStride;
                            auto src = RawDataView<uint16_t>{
                                idxBuffer.data(), idxByteOffset + indexOffset, indexCount, stride
                            };
                            auto indexIdx = size_t{ 0 };
                            for (auto& index : geometry.indices()) {
                                index = static_cast<index_type_t>(*std::next(src.begin(), indexIdx++));
                            }
                        } break;
                        case 5125: { // unsigned int
                            auto stride = idxByteStride == 0 ? sizeof(uint32_t) : idxByteStride;
                            auto src = RawDataView<uint32_t>{
                                idxBuffer.data(), idxByteOffset + indexOffset, indexCount, stride
                            };
                            auto indexIdx = size_t{ 0 };
                            for (auto& index : geometry.indices()) {
                                index = static_cast<index_type_t>(*std::next(src.begin(), indexIdx++));
                            }
                        } break;
                    }
                } // end index rendering

                geometryIdentifiers_.emplace(std::make_tuple(pos, nor, ind), geometry.id());
            }

            if constexpr (std::is_same_v<std::decay_t<decltype(std::get<0>(t))>,
                                         std::vector<gltf::Reader::Primitive>>) {
                // TODO:: refactor ...
                auto&& [primitives] = t;

                std::vector<std::pair<uint32_t, uint32_t>> subMeshData = {};

                subMeshData.reserve(primitives.size());

                for (auto&& [pos, nor, ind] : primitives) {
                    uint32_t vertexCount = 0;
                    uint32_t indexCount = 0;

                    {
                        auto const& accessor = reader.accessors()[pos];
                        auto&& [buffViewIdx, byteOffset, componentType, normalized, count, type] = accessor;

                        vertexCount = count;
                    }

                    {
                        auto const& accessor = reader.accessors()[ind];
                        auto&& [buffViewIdx, byteOffset, componentType, normalized, count, type] = accessor;

                        indexCount = count;
                    }

                    subMeshData.emplace_back(std::make_pair(indexCount, vertexCount));
                }

                auto& meshSystem = systems_.get<systems::MeshSystem>();
                // auto& mesh = meshSystem.createMesh();
            }
        });
    }

    return *this;
}
}

CYCLONITE_APP(examples::GLTFViewer)
