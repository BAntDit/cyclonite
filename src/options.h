//
// Created by bantdit on 1/19/19.
//

#ifndef CYCLONITE_OPTIONS_H
#define CYCLONITE_OPTIONS_H

#include <boost/foreach.hpp>
#include <boost/program_options.hpp>
#include <cstdint>
#include <iostream>
#include <string>
#include <vector>

namespace cyclonite {
class Options
{
public:
    Options(int argc, const char* argv[]) noexcept;

    template<typename OptionLayoutConfigurator>
    void parse(OptionLayoutConfigurator&& optionLayoutConfigurator);

    [[nodiscard]] auto has(std::string const& optionName) const -> bool;

    template<typename T>
    [[nodiscard]] auto get(std::string const& optionName) const -> T;

private:
    int argc_;
    char const** argv_;
    boost::program_options::variables_map variables_;
};

template<typename OptionLayoutConfigurator>
void Options::parse(OptionLayoutConfigurator&& optionLayoutConfigurator)
{
    static_assert(std::is_invocable_v<optionLayoutConfigurator, boost::program_options::options_description_easy_init>);

    boost::program_options::options_description commandLineOptions{ "commandLineOptions" };

    optionLayoutConfigurator(commandLineOptions.add_options());

    if (argc_ > 0) {
        boost::program_options::store(boost::program_options::parse_command_line(argc_, argv_, commandLineOptions),
                                      variables_);
    }

    boost::program_options::notify(variables_);

    if (variables_.count("help") > 0)
        std::cout << commandLineOptions << std::endl;
}

template<typename T>
auto Options::get(std::string const& optionName) const -> T
{
    if (variables_.count(optionName) == 0)
        throw std::runtime_error("option does not exist");

    return variables_[optionName].as<T>();
}
}

#endif // CYCLONITE_OPTIONS_H
