//
// Created by bantdit on 8/2/20.
//

#include "viewer.h"
#include "appConfig.h"
#include "controller.h"
#include "model.h"
#include "view.h"

using namespace cyclonite;
using namespace easy_mp;

namespace examples::viewer {

Viewer::Viewer()
  : root_{ std::make_unique<cyclonite::Root>() }
  , model_{ nullptr }
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
        workspaceBuilder.createNode(
          node_type_register_t::node_key_t<MainNodeConfig>{},
          [width = width, height = height](auto&& nodeBuilder) -> cyclonite::compositor::Node<MainNodeConfig> {
              return nodeBuilder.setOutputResolution(width, height)
                .setRenderTargetDepthProperties(
                  cyclonite::compositor::render_target_output<
                    type_list<cyclonite::compositor::render_target_candidate_t<VK_FORMAT_D32_SFLOAT>>,
                    RenderTargetOutputSemantic::UNDEFINED>{})
                .setRenderTargetColorProperties(
                  cyclonite::compositor::render_target_output<
                    type_list<cyclonite::compositor::render_target_candidate_t<VK_FORMAT_R16G16B16A16_SFLOAT>>,
                    cyclonite::RenderTargetOutputSemantic::VIEW_SPACE_NORMALS,
                    true>{},
                  cyclonite::compositor::render_target_output<
                    type_list<cyclonite::compositor::render_target_candidate_t<VK_FORMAT_R8G8B8A8_UNORM>>,
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

        workspaceBuilder.createNode(
          node_type_register_t::node_key_t<SurfaceNodeConfig>{},
          [width = width, height = height](auto&& nodeBuilder) -> cyclonite::compositor::Node<SurfaceNodeConfig> {
              return nodeBuilder.template createInputLinks<1>()
                .setOutputResolution(width, height)
                .setRenderTargetColorProperties(
                  cyclonite::compositor::render_target_output<
                    type_list<cyclonite::compositor::render_target_candidate_t<VK_FORMAT_B8G8R8A8_SRGB>>,
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
                .template setInputs<cyclonite::RenderTargetOutputSemantic::VIEW_SPACE_NORMALS,
                                    cyclonite::RenderTargetOutputSemantic::ALBEDO>(0)
                .build();
          });

        return workspaceBuilder.build();
    });

    model_ = std::make_unique<Model>();
    model_->init(*root_, "./BoxAnimated.gltf", workspace);

    view_ = std::make_unique<View>();
    view_->init(root_->taskManager(), root_->device(), workspace);

    controller_ = std::make_unique<Controller>();
    controller_->init(root_->input(), width, height);

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

        view_->draw(root_->device());
    }

    return *this;
}

void Viewer::done()
{
    assert(view_);
    auto& workspace = view_->workspace();

    assert(workspace);
    {
        auto& node = workspace->get(node_type_register_t::node_key_t<MainNodeConfig>{});
        node.systems().get<systems::RenderSystem>().finish();
    }

    {
        auto& node = workspace->get(node_type_register_t::node_key_t<SurfaceNodeConfig>{});
        node.systems().get<systems::RenderSystem>().finish();
    }
}
}

CYCLONITE_APP(examples::viewer::Viewer)
