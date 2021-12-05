//
// Created by bantdit on 8/2/20.
//

#include "viewer.h"
#include "controller.h"
#include "model.h"
#include "view.h"
#include <chrono>

using namespace cyclonite;
using namespace easy_mp;

namespace examples::viewer {
namespace {
struct MainNodeConfig : public cyclonite::DefaultConfigs
{};

struct SurfaceNodeConfig : public cyclonite::DefaultConfigs
{
    using ecs_config_t = EcsConfig<type_list<components::Mesh>,
                                   type_list<components::MeshStorage<1>>,
                                   type_list<systems::RenderSystem, systems::MeshSystem, systems::UniformSystem>,
                                   value_cast(UpdateStage::COUNT)>;
};
}

Viewer::Viewer()
  : root_{ std::make_unique<cyclonite::Root>() }
  , model_{ nullptr }
  , view_{ nullptr }
  , controller_{ nullptr }
{}

auto Viewer::init(cyclonite::Options options) -> Viewer&
{
    options.parse([](auto&& layout) -> void {
        layout("help, h",
               "show help")("device-name, d", boost::program_options::value<std::string>(), "specifies device name");
    });

    auto deviceId =
      options.has("device-name") ? root_->getDeviceId(options.get<std::string>("device-name")) : root_->getDeviceId();

    root_->init(deviceId);

    uint32_t width = 1024;
    uint32_t height = 768;

    auto&& workspace = root_->createWorkspace([=](auto&& workspaceBuilder) -> cyclonite::compositor::Workspace {
        workspaceBuilder.template createNode<MainNodeConfig>(
          [width = width, height = height](auto&& nodeBuilder) -> cyclonite::compositor::Node<MainNodeConfig> {
              return nodeBuilder.setOutputResolution(width, height)
                .setRenderTargetDepthProperties(
                  cyclonite::compositor::render_target_output<
                    type_list<cyclonite::compositor::render_target_candidate_t<VK_FORMAT_D32_SFLOAT>>,
                    RenderTargetOutputSemantic::UNDEFINED>{})
                .setRenderTargetColorProperties(
                  cyclonite::compositor::render_target_output<
                    type_list<cyclonite::compositor::render_target_candidate_t<VK_FORMAT_R16G16B16_SFLOAT>>,
                    cyclonite::RenderTargetOutputSemantic::VIEW_SPACE_NORMALS,
                    true>{},
                  cyclonite::compositor::render_target_output<
                    type_list<cyclonite::compositor::render_target_candidate_t<VK_FORMAT_R8G8B8_UINT>>,
                    cyclonite::RenderTargetOutputSemantic::ALBEDO,
                    true>{})
                .addPass(cyclonite::compositor::PassType::SCENE,
                         std::array{ uint32_t{ 0 }, uint32_t{ 1 } },
                         0,
                         VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                         VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                         0,
                         VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
                         true,
                         0)
                .build();
          });

        workspaceBuilder.template createNode<SurfaceNodeConfig>(
          [width = width, height = height](auto&& nodeBuilder) -> cyclonite::compositor::Node<SurfaceNodeConfig> {
              return nodeBuilder.setOutputResolution(width, height)
                .setRenderTargetColorProperties(
                  cyclonite::compositor::render_target_output<
                    type_list<cyclonite::compositor::render_target_candidate_t<VK_FORMAT_R8G8B8_UINT>>,
                    cyclonite::RenderTargetOutputSemantic::FINAL_SRGB_COLOR>{})
                .setSurface(
                  cyclonite::WindowProperties{ "test-viewer", 0, 0, width, height, false },
                  cyclonite::compositor::surface_parameters<
                    type_list<cyclonite::compositor::color_space_candidate_t<VK_COLOR_SPACE_SRGB_NONLINEAR_KHR>>,
                    type_list<cyclonite::compositor::present_mode_candidate_t<VK_PRESENT_MODE_MAILBOX_KHR>,
                              cyclonite::compositor::present_mode_candidate_t<VK_PRESENT_MODE_FIFO_KHR>>>{})
                .addPass(cyclonite::compositor::PassType::SCREEN,
                         std::array{ uint32_t{ 0 } },
                         0,
                         VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                         VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
                         0,
                         VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
                         false,
                         0)
                .setInputs<cyclonite::RenderTargetOutputSemantic::VIEW_SPACE_NORMALS,
                           cyclonite::RenderTargetOutputSemantic::ALBEDO>(0)
                .build();
          });

        return workspaceBuilder.build();
    });

    view_ = std::make_unique<View>();
    // TODO:: create workspace in the view init

    model_ = std::make_unique<Model>();
    model_->init(workspace);

    // TODO:: sync
    // one node for whole scene normal pass
    // one node for surface pass

    /*
    view_ = std::make_unique<View>();
    view_->init(root_->device(), root_->taskManager(), systems_);

    model_ = std::make_unique<Model>();
    model_->init(root_->device(), entities_, systems_, "./scene.gltf");

    controller_ = std::make_unique<Controller>();
    controller_->init(root_->input(), systems_);
    */

    return *this;
}

auto Viewer::run() -> Viewer&
{
    auto start = std::chrono::high_resolution_clock::now();

    while (controller_->alive()) {
        auto end = std::chrono::high_resolution_clock::now();

        auto dt = std::chrono::duration<real, std::ratio<1>>{ end - start }.count();

        start = end;

        controller_->update(*model_, dt);
        view_->draw(*model_);
    }

    return *this;
}

void Viewer::done()
{
    // systems_.get<systems::RenderSystem>().finish();
}
}

CYCLONITE_APP(examples::viewer::Viewer)
