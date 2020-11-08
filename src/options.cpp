//
// Created by bantdit on 1/19/19.
//

#include "options.h"
#include <fstream>
#include <iomanip>
#include <iostream>
#include <istream>
#include <nlohmann/json.hpp>

namespace po = boost::program_options;

namespace cyclonite {
Options::Options(int argc, const char* argv[]) noexcept
  : argc_{ argc }
  , argv_{ argv }
  , variables_{}
{}

auto Options::has(std::string const& optionName) const -> bool
{
    return variables_.count(optionName) > 0;
}
}
