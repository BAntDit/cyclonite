//
// Created by bantdit on 1/19/19.
//

#ifndef CYCLONITE_OPTIONS_H
#define CYCLONITE_OPTIONS_H

#include <boost/foreach.hpp>
#include <boost/program_options.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/property_tree/ptree.hpp>
#include <cstdint>
#include <string>
#include <vector>

namespace cyclonite {
class Options
{
public:
    struct WindowProperties
    {
        std::string title;
        uint32_t left;
        uint32_t top;
        uint32_t width;
        uint32_t height;
        bool fullscreen;
    };

    explicit Options(int argc = 0, const char* argv[] = {});

private:
    void parseWindowProperties();

private:
    std::string config_;
    std::string deviceName_;
    boost::property_tree::ptree properties_;
    std::vector<WindowProperties> windows_;
};
}

#endif // CYCLONITE_OPTIONS_H
