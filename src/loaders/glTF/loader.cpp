//
// Created by bantdit on 1/20/19.
//

#include "loader.h"
#include "internal/getOptional.h"
#include "internal/readArray.h"
#include <regex>

namespace cyclonite::loaders::gltf {
Loader::Loader(multithreading::TaskManager& taskManager)
  : basePath_{}
{}

void Loader::_parseAsset(json& input)
{
    auto it = input.find(u8"asset");

    if (it == input.end()) {
        throw std::runtime_error("glTF json must contain asset object");
    }

    json& asset = (*it);

    if (!asset.is_object()) {
        throw std::runtime_error("glTF json must contain asset object");
    }

    if (!_testVersion(asset)) {
        throw std::runtime_error("asset has unsupported glTF version");
    }
}

bool Loader::_testVersion(json& asset)
{
    // From spec: https://github.com/KhronosGroup/glTF/tree/master/specification/2.0#asset
    //
    // Implementation Note: Client implementations should first check whether a minVersion property is specified
    // and ensure both major and minor versions can be supported.
    // If no minVersion is specified, then clients should check the version property
    // and ensure the major version is supported.

    uint16_t minimalMajorVersion = 2;
    uint16_t supportedMajorVersion = 2;
    uint16_t supportedMinorVersion = 0;

    // check minVersion at first
    {
        // it must contains minor and major version at once, rest part of version is optional
        std::regex re(u8"^(\\d+).(\\d+)($|.\\d+$|(.\\d+.\\d+$))");
        std::smatch matches;

        auto it = asset.find(u8"minVersion");

        if (it != asset.end()) {
            if (!it.value().is_string()) {
                throw std::runtime_error("glTF asset has wrong the minVersion value type");
            }

            auto minVersion = it.value().get<std::string>();

            if (std::regex_match(minVersion, matches, re)) {
                assert(matches.size() >= 2);

                auto majorVersion = static_cast<uint16_t>(strtol(matches[0].str().c_str(), nullptr, 10));
                auto minorVersion = static_cast<uint16_t>(strtol(matches[1].str().c_str(), nullptr, 10));

                if (supportedMajorVersion >= majorVersion && majorVersion >= minimalMajorVersion) {
                    if (supportedMajorVersion == majorVersion && supportedMinorVersion < minorVersion) {
                        return false;
                    }

                    return true;
                }

                return false;
            }

            throw std::runtime_error("glTF asset has wrong the minVersion format");
        }
    }

    // check version
    {
        auto it = asset.find(u8"version");

        // if there is no minVersion, the target version must be
        // From spec: https://github.com/KhronosGroup/glTF/tree/master/specification/2.0#asset
        // The asset object must contain glTF version which specifies the target glTF version of the asset
        if (it == asset.end()) {
            throw std::runtime_error("glTF asset object does not contain target glTF version");
        }

        if (!it.value().is_string()) {
            throw std::runtime_error("glTF asset has wrong the version value type");
        }

        auto version = it.value().get<std::string>();

        // it must contain major version only, rest part is optional
        std::regex re(u8"^(\\d+)($|.(\\d+))($|.\\d+$|(.\\d+.\\d+$))");
        std::smatch matches;

        if (std::regex_match(version, matches, re)) {
            assert(!matches.empty());

            auto majorVersion = static_cast<uint16_t>(strtol(matches[0].str().c_str(), nullptr, 10));

            if (majorVersion >= minimalMajorVersion) {
                return true;
            }

            return false;
        }

        throw std::runtime_error("glTF asset has wrong the version format");
    }

    std::terminate();
}

auto Loader::_parseNode(json& _node) -> GLTFNode
{
    GLTFNode node;

    node.name = internal::getOptional(_node, u8"name", std::string{""});
    node.camera = internal::getOptional(_node, u8"camera", std::numeric_limits<size_t>::max());
    node.skin = internal::getOptional(_node, u8"skin", std::numeric_limits<size_t>::max());
    node.mesh = internal::getOptional(_node, u8"mesh", std::numeric_limits<size_t>::max());

    {
        auto it = _node.find(u8"children");

        if (it != _node.end()) {
            auto& _children = *it;

            if (!_children.is_array()) {
                throw std::runtime_error("glTF node.children property must be an array of numbers or undefined");
            }

            if (_children.empty()) {
                throw std::runtime_error("glTF node.children array must have at least one item");
            }

            for (size_t i = 0; i < _children.size(); i++) {
                node.children.push_back(_children.at(i).get<size_t>());
            }
        }
    }

    // TODO:: read matrix...

    return node;
}

void Loader::_parseNodes(json& input)
{
    auto it = input.find(u8"nodes");

    if (it == input.end())
        return;

    auto& _nodes = (*it);

    if (!_nodes.is_array()) {
        throw std::runtime_error("glTF nodes property must be an array of numbers or undefined");
    }

    nodes_.reserve(_nodes.size());

    auto [taskCount, nodesPerTask] = taskManager_->getTaskCount(_nodes.size());

    std::vector<std::future<std::vector<GLTFNode>>> futures(taskCount);

    for (size_t taskIndex = 0; taskIndex < taskCount; taskIndex++) {
        size_t taskStartIndex = taskIndex * nodesPerTask;
        size_t taskEndIndex = std::min((taskIndex + 1) * nodesPerTask, _nodes.size());

        futures[taskIndex] = taskManager_->submit([&, this, taskStartIndex, taskEndIndex]() -> std::vector<GLTFNode> {
            std::vector<GLTFNode> nodes;

            nodes.reserve(taskEndIndex - taskStartIndex);

            for (size_t i = taskStartIndex; i < taskEndIndex; i++) {
                auto& node = _nodes.at(i);

                if (!node.is_object()) {
                    throw std::runtime_error("glTF node should be a json object");
                }

                nodes.emplace_back(this->_parseNode(node));
            }

            return nodes;
        });
    }

    for (auto&& future : futures) {
        auto nodes = future.get();

        for (auto&& node : nodes) {
            nodes_.emplace_back(node);
        }
    }
}
}
