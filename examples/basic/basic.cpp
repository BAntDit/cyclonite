//
// Created by bantdit on 12/31/19.
//

#include "basic.h"

namespace examples {
Basic::Basic()
  : shutdown_{ false }
  , root_{ std::make_unique<cyclonite::Root>() }
  , entities_{}
  , systems_{ &entities_ }
  , cameraEntity_{}
{
}

auto Basic::init(cyclonite::Options const& options) -> Basic&
{
    using namespace cyclonite;
    using namespace easy_mp;

    root_->init(options);
    root_->input().keyDown += cyclonite::Event<SDL_Keycode, uint16_t>::EventHandler(this, &Basic::onKeyDown);

    // init systems (begin)

    auto& renderSystem = systems_.get<systems::RenderSystem>();

    {
        Options::WindowProperties windowProperties{};

        windowProperties.title = "basic.example";
        windowProperties.fullscreen = false;
        windowProperties.left = 0; // SDL_WINDOWPOS_UNDEFINED;
        windowProperties.top = 0;  // SDL_WINDOWPOS_UNDEFINED;
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

    auto& transformSystem = systems_.get<systems::TransformSystem>();
    transformSystem.init(128);

    auto& meshSystem = systems_.get<systems::MeshSystem>();
    meshSystem.init(root_->device(), 1, 1, 1, 1);

    auto& cameraSystem = systems_.get<systems::CameraSystem>();
    cameraSystem.init();

    auto& uniformSystem = systems_.get<systems::UniformSystem>();
    uniformSystem.init(root_->device());
    // init systems (done)

    // root entity
    auto rootEntity = entities_.create();
    transformSystem.create(entities_, enttx::Entity{}, rootEntity, cyclonite::mat4{ 1.f });

    // child with mesh
    {
        auto entities = entities_.create(std::array<enttx::Entity, 1>{});

        cyclonite::vec3 pos = cyclonite::vec3{ 0.f, -1.f, -4.f };

        transformSystem.create(entities_,
                               rootEntity,
                               entities[0],
                               pos,
                               cyclonite::vec3{ 1.f },
                               glm::angleAxis(glm::pi<boost::float32_t>() / 6.0f, cyclonite::vec3{ 0.f, 1.f, 0.f }));

        entities_.assign<cyclonite::components::Mesh>(entities[0]);
    }

    // camera
    {
        auto cameraEntity = entities_.create();

        transformSystem.create(entities_,
                               rootEntity,
                               cameraEntity,
                               cyclonite::vec3{ 0.0f, 0.0f, 2.f },
                               cyclonite::vec3{ 1.f },
                               cyclonite::quat{ 1.f, 0.f, 0.f, 0.f });

        entities_.assign<cyclonite::components::Camera>(
          cameraEntity,
          cyclonite::components::Camera::PerspectiveProjection{ 1.f, glm::pi<boost::float32_t>() / 4.f, 0.1f, 100.f });

        cameraEntity_ = cameraEntity;
    }

    return *this;
}

auto Basic::run() -> Basic&
{
    while (!shutdown_) {
        root_->input().pollEvent();

        systems_.update(cameraEntity_, 0.f);
    }

    return *this;
}

void Basic::done()
{
    systems_.get<systems::RenderSystem>().finish();
}

void Basic::onKeyDown(SDL_Keycode keyCode, uint16_t mod)
{
    (void)mod;

    if (keyCode == SDLK_ESCAPE)
        shutdown_ = true;
}
}

CYCLONITE_APP(examples::Basic)
