
#include "basicExample.h"
#include <boost/foreach.hpp>
#include <boost/program_options.hpp>
#include <cassert>

#include "gfx/queueSubmissionRecorder.h"
#include "gfx/vulkan/vkQueueSubmissionManager.h"

namespace examples {
BasicExample::BasicExample()
  : cyclonite::EventReceivable{}
  , root_{}
  , submissionManager_{}
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
    auto renderPassRef = cyclonite::core::ResourceSharedRef{
        renderPassBuilder.setDevice(deviceRef).setRenderWindow(renderWindowRef).build()
    };

    submissionManager_ = std::make_unique<cyclonite::gfx::QueueSubmissionManager>(deviceRef);

    deviceRef_ = deviceRef;
    renderPassRef_ = renderPassRef;

    root_.input().quit += cyclonite::Event<>::EventHandler(this, &BasicExample::onQuit);
    root_.input().keyDown += cyclonite::Event<SDL_Keycode, uint16_t>::EventHandler(this, &BasicExample::onKeyDown);

    return *this;
}

struct RenderTask
{
    explicit RenderTask(cyclonite::gfx::QueueSubmissionManager* submissionManager,
                        cyclonite::core::ResourceSharedRef renderPassRef)
      : renderPassRef_{ std::move(renderPassRef) }
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
    }

    cyclonite::core::ResourceSharedRef renderPassRef_;
    cyclonite::gfx::QueueSubmissionManager* submissionManager_;
};

auto BasicExample::run() -> BasicExample&
{
    auto& taskManager = root_.taskManager();

    while (!shutdown_) {
        root_.input().pollEvent();

        taskManager
          .submitTask(RenderTask{ submissionManager_.get(), renderPassRef_ },
                      cyclonite::multithreading::Purpose::Render)
          .get();
    }

    return *this;
}

void BasicExample::done()
{
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
