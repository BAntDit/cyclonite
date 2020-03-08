//
// Created by bantdit on 12/31/19.
//

#include "minimal.h"

namespace examples
{
Minimal::Minimal()
  : shutdown_{ false }
  , root_{ std::make_unique<cyclonite::Root>() }
  , entities_{}
  , systems_{ &entities_ }
  , cameraEntity_{}
{}

auto Minimal::init(cyclonite::Options const& options) -> Minimal& {
    root_->init(options);

    root_->input().keyDown += cyclonite::Event<SDL_KeyboardEvent>::EventHandler(this, &Minimal::onKeyDown);

    auto rootEntity = entities_.create();
    auto entities = entities_.create(std::array<enttx::Entity, 32>{});

    auto& transformSystem = systems_.get<cyclonite::systems::TransformSystem>();
    transformSystem.init(entities.size() + 1);

    auto& meshSystem = systems_.get<cyclonite::systems::MeshSystem>();
    meshSystem.init(root_->device());

    auto& cameraSystem = systems_.get<cyclonite::systems::CameraSystem>();
    cameraSystem.init(root_->device());

    auto& transferSystem = systems_.get<cyclonite::systems::TransferSystem>();
    transferSystem.init(root_->device());

    transformSystem.create(entities_, enttx::Entity{}, rootEntity, cyclonite::mat4{1.f});

    boost::float32_t xShift = 0.25f;
    boost::float32_t yShift = 0.25f;
    boost::float32_t zShift = 0.25f;

    cyclonite::vec3 pos = cyclonite::vec3{-1.f, 0.f, 0.f};

    uint8_t i = 0;

    for (auto&& entity : entities) {
        transformSystem.create(
          entities_, rootEntity, entity, pos, cyclonite::vec3{1.f}, cyclonite::quat{1.f, 0.f, 0.f, 0.f});

        if (i % 2 == 0)
            entities_.assign<cyclonite::components::Mesh>(entity);

        pos.x += xShift;
        pos.y += static_cast<boost::float32_t>(i++ / 4) * yShift;
        pos.z -= static_cast<boost::float32_t>(i++ / 4) * zShift;
    }

    auto cameraEntity = entities_.create();

    transformSystem.create(
      entities_,
      rootEntity,
      cameraEntity,
      cyclonite::vec3{0.0f, 0.0f, 2.f},
      cyclonite::vec3{1.f},
      cyclonite::quat{1.f, 0.f, 0.f, 0.f});

    entities_.assign<cyclonite::components::Camera>(
      cameraEntity,
      cyclonite::components::Camera::PerspectiveProjection{
          1.f, glm::pi<boost::float32_t>() / 2.f, 0.1f, 100.f });

    cameraEntity_ = cameraEntity;

    return *this;
}

auto Minimal::run() -> Minimal& {
    cyclonite::Options::WindowProperties windowProperties{};

    windowProperties.title = "minimal.example";
    windowProperties.fullscreen = false;
    windowProperties.left = 0x1FFF;
    windowProperties.top = 0x1FFF;
    windowProperties.width = 512;
    windowProperties.height = 512;

    cyclonite::VulkanRenderer vulkanRenderer{ root_->device() };

    cyclonite::RenderPass renderPass{
        root_->device(),
        windowProperties,
        cyclonite::render_target_output<
            easy_mp::type_list<cyclonite::render_target_output_candidate<VK_FORMAT_D32_SFLOAT>>>{},
        cyclonite::render_target_output<
            easy_mp::type_list<
                cyclonite::render_target_output_candidate<VK_FORMAT_B8G8R8A8_UNORM, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR>>,
                cyclonite::RenderTargetOutputSemantic::DEFAULT>{},
        VkClearDepthStencilValue{ 1.0f, 0 },
        VkClearColorValue{ { 0.0f, 0.0f, 0.0f, 1.0f } },
        std::array{ VK_PRESENT_MODE_MAILBOX_KHR, VK_PRESENT_MODE_FIFO_KHR } };

    while(!shutdown_) {
        root_->input().pollEvent();

        systems_.update(renderPass.frame(), cameraEntity_, 0.f);

        vulkanRenderer.renderOneFrame(renderPass);
    }

    return *this;
}

void Minimal::done() {}

void Minimal::onKeyDown(SDL_KeyboardEvent event) {
    if (event.keysym.sym == SDLK_ESCAPE)
        shutdown_ = true;
}
}

CYCLONITE_APP(examples::Minimal)

