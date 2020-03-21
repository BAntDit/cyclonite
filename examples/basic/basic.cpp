//
// Created by bantdit on 12/31/19.
//

#include "basic.h"

namespace examples
{
Basic::Basic()
  : shutdown_{ false }
  , root_{ std::make_unique<cyclonite::Root>() }
  , renderPass_{ nullptr }
  , renderer_{ nullptr }
  , entities_{}
  , systems_{ &entities_ }
  , cameraEntity_{}
{}

auto Basic::init(cyclonite::Options const& options) -> Basic& {
    root_->init(options);
    root_->input().keyDown += cyclonite::Event<SDL_Keycode, uint16_t>::EventHandler(this, &Basic::onKeyDown);

    {
        cyclonite::Options::WindowProperties windowProperties{};

        windowProperties.title = "basic.example";
        windowProperties.fullscreen = false;
        windowProperties.left = SDL_WINDOWPOS_CENTERED;
        windowProperties.top = SDL_WINDOWPOS_CENTERED;
        windowProperties.width = 512;
        windowProperties.height = 512;

        renderPass_ = std::make_unique<RenderPass>(root_->device(),
          windowProperties,
          cyclonite::render_target_output<
            easy_mp::type_list<cyclonite::render_target_output_candidate<VK_FORMAT_D32_SFLOAT>>>{},
            cyclonite::render_target_output<
              easy_mp::type_list<
                cyclonite::render_target_output_candidate<VK_FORMAT_B8G8R8A8_UNORM, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR>>,
            cyclonite::RenderTargetOutputSemantic::DEFAULT>{},
          VkClearDepthStencilValue{ 1.0f, 0 },
          VkClearColorValue{ { 0.0f, 0.0f, 0.0f, 1.0f } },
          std::array{ VK_PRESENT_MODE_MAILBOX_KHR, VK_PRESENT_MODE_FIFO_KHR }
        );
    }

    renderer_ = std::make_unique<VulkanRenderer>(root_->device(), root_->taskManager());

    auto rootEntity = entities_.create();

    auto entities = entities_.create(std::array<enttx::Entity, 1>{});

    auto& transformSystem = systems_.get<cyclonite::systems::TransformSystem>();
    transformSystem.init(128);

    auto& meshSystem = systems_.get<cyclonite::systems::MeshSystem>();
    meshSystem.init(root_->device());

    auto& cameraSystem = systems_.get<cyclonite::systems::CameraSystem>();
    cameraSystem.init(root_->device());

    auto& transferSystem = systems_.get<cyclonite::systems::TransferSystem>();
    transferSystem.init(root_->device());

    transformSystem.create(entities_, enttx::Entity{}, rootEntity, cyclonite::mat4{1.f});

    cyclonite::vec3 pos = cyclonite::vec3{0.f, -1.f, -4.f};

    transformSystem.create(
      entities_,
      rootEntity,
      entities[0],
      pos,
      cyclonite::vec3{1.f},
      glm::angleAxis(glm::pi<boost::float32_t>() / 6.0f, cyclonite::vec3{0.f, 1.f, 0.f})
    );

    entities_.assign<cyclonite::components::Mesh>(entities[0]);

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
          1.f, glm::pi<boost::float32_t>() / 4.f, 0.1f, 100.f });

    cameraEntity_ = cameraEntity;

    return *this;
}

auto Basic::run() -> Basic& {
    while(!shutdown_) {
        root_->input().pollEvent();

        systems_.update(renderPass_->frame(), cameraEntity_, 0.f);

        renderer_->renderOneFrame(*renderPass_);
    }

    return *this;
}

void Basic::done() {
    renderer_->finish();
}

void Basic::onKeyDown(SDL_Keycode keyCode, uint16_t mod) {
    (void)mod;

    if (keyCode == SDLK_ESCAPE)
        shutdown_ = true;
}
}

CYCLONITE_APP(examples::Basic)
