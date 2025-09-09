
#include "commandLine.h"

namespace cyclonite {
CommandLine::CommandLine(int argc, const char* argv[])
  : argc_{ static_cast<uint32_t>(argc) }
  , argv_{ argv }
  , arguments_{}
{
    for (auto i = size_t{ 0 }, count = static_cast<size_t>(argc_); i < argc_; i++) {
        arguments_.emplace(argv_[i]);
    }
}
}
