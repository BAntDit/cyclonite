
#include "basicExample.h"
#include <boost/foreach.hpp>
#include <boost/program_options.hpp>

namespace examples {
auto BasicExample::init(cyclonite::CommadnLine const& commandLine) -> BasicExample&
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

    auto deviceId = commandLineVariables["device-id"].as<uint32_t>();

    root_.init("basic-example", deviceId);

    return *this;
}

auto BasicExample::run() -> BasicExample&
{
    return *this;
}

void BasicExample::done() {
    root_.reset();
}
}