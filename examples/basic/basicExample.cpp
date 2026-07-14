
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
  , scroll_{ 0.f }
  , fade_{ .9f }
  , distance_{ 0.f }
  , distanceMin_{ .5f }
  , distanceMax_{ 10.f }
  , azimuth_{ 0.f }
  , polar_{ 0.f }
  , rotate_{}
  , rotationStart_{}
  , target_{}
  , time_{}
  , isInRotation_{ false }
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
    root_.input().mouseButtonDown += cyclonite::EventHandler(this, &BasicExample::onMouseButtonDown);
    root_.input().mouseButtonUp += cyclonite::EventHandler(this, &BasicExample::onMouseButtonUp);
    root_.input().mouseMotion += cyclonite::EventHandler(this, &BasicExample::onMouseMotion);
    root_.input().mouseWheel += cyclonite::EventHandler(this, &BasicExample::onMouseWheel);

    renderer_.init(
      deviceRef_, windowRef_, vertexShaderRef, fragmentShaderRef, materialRef_, device.queueSubmissionManager());

    time_ = std::chrono::high_resolution_clock::now();

    return *this;
}

auto BasicExample::run() -> BasicExample&
{
    auto frameStartTime = std::chrono::high_resolution_clock::now();
    auto dt = std::chrono::duration<cyclonite::real, std::ratio<1>>{ frameStartTime - time_ }.count();
    time_ = frameStartTime;

    root_.input().pollEvent();

    distance_ += scroll_;
    distance_ = std::max(distanceMin_, std::min(distanceMax_, distance_));

    scroll_ -= scroll_ * (1.f - fade_);
    scroll_ = abs(scroll_) < 0.1e-3f ? 0.f : scroll_;

    azimuth_ += rotate_.x * dt;
    polar_ += rotate_.y * dt;

    rotate_.x -= rotate_.x * (1.f - fade_);
    rotate_.y -= rotate_.y * (1.f - fade_);

    constexpr auto pi = std::numbers::pi_v<cyclonite::real>;
    polar_ = std::max((pi / 2.f), std::min(pi - 0.1f, polar_));

    auto pos = cyclonite::vec3{ distance_ * sinf(polar_) * cosf(azimuth_),
                                distance_ * cosf(polar_),
                                distance_ * sinf(azimuth_) * sinf(polar_) };

    auto up = cyclonite::vec3{ .0f, 1.f, 0.f };
    auto fw = glm::normalize(-pos);
    auto lf = glm::normalize(glm::cross(up, fw));

    up = glm::normalize(glm::cross(fw, lf));

    auto camera =
      cyclonite::components::Camera{ cyclonite::components::Camera::PerspectiveProjection{ 1.f, 45.f, 0.1f, 100.f } };

    auto cameraMatrix =
      cyclonite::mat4{ cyclonite::vec4{ lf.x, lf.y, lf.z, 0.0f },
                       cyclonite::vec4{ up.x, up.y, up.z, 0.0f },
                       cyclonite::vec4{ fw.x, fw.y, fw.z, 0.0f },
                       cyclonite::vec4{ target_.x - pos.x, target_.y - pos.y, target_.z - pos.z, 1.0f } };

    auto transform = cyclonite::components::Transform{ cameraMatrix };

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

void BasicExample::onMouseButtonDown(uint8_t button, uint8_t clicks, float x, float y)
{
    if (button == SDL_BUTTON_LEFT && clicks == 1) {
        isInRotation_ = true;
        rotationStart_.x = x;
        rotationStart_.y = y;
    }
}

void BasicExample::onMouseButtonUp(uint8_t button, float x, float y)
{
    (void)button;
    (void)x;
    (void)y;

    isInRotation_ = false;
}

void BasicExample::onMouseMotion(float x, float y)
{
    if (isInRotation_) {
        constexpr auto speed = cyclonite::real{ 30.f };
        constexpr auto pi = std::numbers::pi_v<cyclonite::real>;

        rotate_.x = pi * 2.f * speed * (static_cast<cyclonite::real>(x) - rotationStart_.x);
        rotate_.y = pi * 2.f * speed * (static_cast<cyclonite::real>(y) - rotationStart_.y);

        rotationStart_.x = static_cast<cyclonite::real>(x);
        rotationStart_.y = static_cast<cyclonite::real>(y);
    }
}

void BasicExample::onMouseWheel(float y)
{
    if (y > 0) {
        scroll_ -= 0.05f;
    } else if (y < 0) {
        scroll_ += 0.05f;
    }
}
}
