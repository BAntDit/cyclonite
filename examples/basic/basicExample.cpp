
#include "basicExample.h"
#include <boost/foreach.hpp>
#include <boost/program_options.hpp>
#include <cassert>

namespace examples {
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
                                            .addFormatCandidate(cyclonite::gfx::Format::R8G8B8A8_SRGB)
                                            .addPresentModeCandidate(cyclonite::gfx::PresentMode::MailBox)
                                            .addPresentModeCandidate(cyclonite::gfx::PresentMode::FiFo)
                                            .build() };

    auto renderPassBuilder = cyclonite::gfx::RenderPassBuilder{};
    auto renderPassRef = renderPassBuilder.setDevice(deviceRef).setRenderWindow(renderWindowRef).build();

    return *this;
}

auto BasicExample::run() -> BasicExample&
{
    return *this;
}

void BasicExample::done()
{
    root_.reset();
}
}