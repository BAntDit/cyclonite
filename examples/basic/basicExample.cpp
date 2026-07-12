
#include "basicExample.h"
#include "components/camera.h"
#include "components/transform.h"
#include "gfx/queueSubmissionRecorder.h"
#include <boost/foreach.hpp>
#include <boost/program_options.hpp>
#include <boost/uuid/random_generator.hpp>
#include <cassert>
#include <iostream>

namespace examples {
BasicExample::BasicExample()
  : cyclonite::EventReceivable{}
  , root_{}
  , deviceRef_{}
  , windowRef_{}
  , renderer_{}
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

    auto vertexShaderRef = root_.resourceManager().getResource(defaultResourceGroup_, "testBox.vs.hlsl.sm.bin");
    assert(vertexShaderRef.valid());

    auto fragmentShaderRef = root_.resourceManager().getResource(defaultResourceGroup_, "testBox.fs.hlsl.sm.bin");
    assert(fragmentShaderRef.valid());

    auto renderWindowBuilder = cyclonite::gfx::RenderWindowBuilder{};
    auto renderWindowRef =
      cyclonite::core::ResourceSharedRef{ renderWindowBuilder.setDevice(deviceRef)
                                            .setTitle("basic.example")
                                            .setResolution(1024, 768)
                                            .addFormatCandidate(cyclonite::gfx::Format::B8G8R8A8_SRGB)
                                            .addPresentModeCandidate(cyclonite::gfx::PresentMode::MailBox)
                                            .addPresentModeCandidate(cyclonite::gfx::PresentMode::FiFo)
                                            .build() };

    auto materialUuid = boost::uuids::random_generator()();
    auto testMaterialRef =
      root_.resourceManager().addResource<cyclonite::Material>(defaultResourceGroup_, "testMaterial", materialUuid);

    materialRef_ = std::move(testMaterialRef);

    deviceRef_ = deviceRef;
    windowRef_ = renderWindowRef;

    root_.input().quit += cyclonite::EventHandler(this, &BasicExample::onQuit);
    root_.input().keyDown += cyclonite::EventHandler(this, &BasicExample::onKeyDown);

    renderer_.init(
      deviceRef_, windowRef_, vertexShaderRef, fragmentShaderRef, materialRef_, device.queueSubmissionManager());

    return *this;
}

auto BasicExample::run() -> BasicExample&
{
    auto camera =
      cyclonite::components::Camera{ cyclonite::components::Camera::PerspectiveProjection{ 1.f, 45.f, 0.1f, 100.f } };

    // TODO:: normal camera position
    auto transform = cyclonite::components::Transform{ cyclonite::vec3{ 0.f },
                                                       cyclonite::vec3{ 1.f },
                                                       cyclonite::quat{ 1.f, 0.0f, 0.0f, 0.f } };

    renderer_.setupPassConstants(transform, camera);

    renderer_.render();

    return *this;
}

void BasicExample::done()
{
    std::cout << "app is done!" << std::endl;

    auto& device = deviceRef_.as<cyclonite::gfx::Device>();

    device.queueSubmissionManager().reset();

    root_.taskManager().stop();

    renderer_.reset();

    materialRef_ = cyclonite::core::ResourceSharedRef{};

    root_.resourceManager().releaseGroup(defaultResourceGroup_);

    windowRef_ = cyclonite::core::ResourceSharedRef{};
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
