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

            // TODO:: read primitives before Meshes (make possible to store geometry before creating mesh components)
            if constexpr (std::is_same_v<std::decay_t<decltype(std::get<0>(t))>, gltf::Reader::Primitive>) {
                auto&& [primitive] = t;
                auto&& [pos, nor, ind] = primitive;

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

                // TODO:: ...
            }

            if constexpr (std::is_same_v<std::decay_t<decltype(std::get<0>(t))>,
                                         std::vector<gltf::Reader::Primitive>>) {
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
