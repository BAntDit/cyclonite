
#include "basicExample.h"
#include <boost/foreach.hpp>
#include <boost/program_options.hpp>
#include <cassert>

#include "gfx/queueSubmissionRecorder.h"
#include "gfx/vulkan/vkQueueSubmissionManager.h"

#include <iostream>
static uint32_t framIndex = 0;

namespace examples {
BasicExample::BasicExample()
  : cyclonite::EventReceivable{}
  , root_{}
  , deviceRef_{}
  , renderPassRef_{}
  , windowRef_{}
  , submissionManager_{}
  , defaultResourceGroup_{ std::numeric_limits<uint32_t>::max() }
  , shutdown_{ false }
{
}

auto BasicExample::init(cyclonite::CommandLine const& commandLine) -> BasicExample&
{
    auto commandLineVariables = boost::program_options::variables_map{};

    auto commandLineArgumentLayout = boost::program_options::options_description{ "arguments" };
    commandLineArgumentLayout.add_options()("help, h", "show help")(
      "device-id, d",
      boost::program_options::value<uint32_t>()->default_value(std::numeric_limits<uint32_t>::max()),
      "specifies device id");

    if (commandLine.argumentCount() > 0) {
        boost::program_options::store(boost::program_options::parse_command_line(commandLine.argumentCount(),
                                                                                 commandLine.rawArguments(),
                                                                                 commandLineArgumentLayout),
                                      commandLineVariables);
    }

    boost::program_options::notify(commandLineVariables);

    if (commandLineVariables.count("help") > 0) {
        std::cout << commandLineArgumentLayout << std::endl;
    }

    root_.init("basic-example");

    auto deviceId = commandLineVariables["device-id"].as<uint32_t>();
    auto deviceRef = cyclonite::core::ResourceSharedRef{ root_.gfxInstance().createDevice(deviceId) };
    assert(deviceRef.valid());

    auto& device = deviceRef.as<cyclonite::gfx::Device>();
    auto& limits = device.limits();
    root_.initTaskManager(limits.dedicatedTransferQueue, limits.dedicatedComputeQueue);

    root_.initResourceManager(deviceRef);

    defaultResourceGroup_ = root_.resourceManager().addResourceGroup();
    root_.resourceManager().load(defaultResourceGroup_, L"./../../src/shaders/").get();

    root_.resourceManager().prepare(defaultResourceGroup_).get();

    auto vertexShaderId = root_.resourceManager().getResource(defaultResourceGroup_, "testTriangle.vs.hlsl.sm.bin");
    assert(vertexShaderId.valid());

    auto fragmentShaderId = root_.resourceManager().getResource(defaultResourceGroup_, "testTriangle.fs.hlsl.sm.bin");
    assert(fragmentShaderId.valid());

    auto renderWindowBuilder = cyclonite::gfx::RenderWindowBuilder{};
    auto renderWindowRef =
      cyclonite::core::ResourceSharedRef{ renderWindowBuilder.setDevice(deviceRef)
                                            .setTitle("basic.example")
                                            .setResolution(1024, 768)
                                            .addFormatCandidate(cyclonite::gfx::Format::B8G8R8A8_SRGB)
                                            .addPresentModeCandidate(cyclonite::gfx::PresentMode::MailBox)
                                            .addPresentModeCandidate(cyclonite::gfx::PresentMode::FiFo)
                                            .build() };

    auto renderPassBuilder = cyclonite::gfx::RenderPassBuilder{};
    auto renderPassRef =
      cyclonite::core::ResourceSharedRef{ renderPassBuilder.setDevice(deviceRef)
                                            .setRenderWindow(renderWindowRef,
                                                             cyclonite::gfx::Color{ 0.f, 1.f, 0.f, 1.f })
                                            .build() };

    submissionManager_ = std::make_unique<cyclonite::gfx::QueueSubmissionManager>(deviceRef);

    deviceRef_ = deviceRef;
    renderPassRef_ = renderPassRef;
    windowRef_ = renderWindowRef;

    root_.input().quit += cyclonite::EventHandler(this, &BasicExample::onQuit);
    root_.input().keyDown += cyclonite::EventHandler(this, &BasicExample::onKeyDown);

    return *this;
}

struct RenderTask
{
    explicit RenderTask(cyclonite::gfx::QueueSubmissionManager* submissionManager,
                        cyclonite::core::ResourceSharedRef renderPassRef,
                        cyclonite::core::ResourceSharedRef windowRef)
      : renderPassRef_{ std::move(renderPassRef) }
      , windowRef_{ std::move(windowRef) }
      , submissionManager_{ submissionManager }
    {
    }

    void operator()()
    {
        auto submissionRef = submissionManager_->acquireQueueSubmission(
          cyclonite::multithreading::Purpose::Render,
          cyclonite::gfx::CommandPoolFlagBits{ cyclonite::gfx::CommandPoolFlags::TRANSIENT });

        auto submissionRecorder = cyclonite::gfx::QueueSubmissionRecorder{};
        submissionRecorder.setQueueSubmission(submissionRef);

        auto batchRecorder = submissionRecorder.addBatch();

        auto commandListRecorder = batchRecorder.addCommandList();

        commandListRecorder.begin(cyclonite::gfx::CommandListUsageFlagBits{});

        assert(renderPassRef_.valid());
        commandListRecorder.beginRenderPass(renderPassRef_);
        commandListRecorder.endRenderPass();

        commandListRecorder.end();

        commandListRecorder.finish();

        batchRecorder.finish();

        submissionRecorder.finish();

        submissionManager_->flush();

        auto& window = windowRef_.as<cyclonite::gfx::RenderWindow>();
        window.present();
    }

    cyclonite::core::ResourceSharedRef renderPassRef_;
    cyclonite::core::ResourceSharedRef windowRef_;
    cyclonite::gfx::QueueSubmissionManager* submissionManager_;
};

auto BasicExample::run() -> BasicExample&
{
    auto& taskManager = root_.taskManager();

    while (!shutdown_) {
        root_.input().pollEvent();

        std::cout << "frame start: " << framIndex << std::endl;

        auto&& future = taskManager.submitTask(RenderTask{ submissionManager_.get(), renderPassRef_, windowRef_ },
                                               cyclonite::multithreading::Purpose::Render);

        future.get();

        std::cout << "frame end: " << framIndex << std::endl;

        framIndex++;
    }

    return *this;
}

void BasicExample::done()
{
    std::cout << "app is done!" << std::endl;

    root_.resourceManager().releaseGroup(defaultResourceGroup_);

    windowRef_ = cyclonite::core::ResourceSharedRef{};
    renderPassRef_ = cyclonite::core::ResourceSharedRef{};
    deviceRef_ = cyclonite::core::ResourceSharedRef{};

    root_.reset();
}

void BasicExample::onKeyDown(SDL_Keycode keyCode, uint16_t mod)
{
    (void)mod;

    if (keyCode == SDLK_ESCAPE)
        onQuit();
}
}
