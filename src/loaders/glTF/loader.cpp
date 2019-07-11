//
// Created by bantdit on 1/20/19.
//

#include "loader.h"
#include "internal/getOptional.h"
#include "internal/readArray.h"
#include <glm/gtc/type_ptr.hpp>
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

void Loader::_readBuffers(json& input) {
    auto it = input.find(u8"buffers");

    if (it == input.end()) {
        return;
    }

    auto& buffers = (*it);

    if (!buffers.is_array()) {
        throw std::runtime_error("glTF.buffers property should be an array or undefined");
    }

    auto countBuffers = buffers.size();

    if (countBuffers == 0) {
        return;
    }

    std::vector<std::vector<std::byte>> buffers_;

    std::vector<std::future<void>> futures(countBuffers);

    buffers_.resize(countBuffers);

    for (size_t i = 0; i < countBuffers; i++) {
        auto& jsonBuffer = buffers.at(i);

        if (!jsonBuffer.is_object()) {
            throw std::runtime_error("glTF json buffer should be an object");
        }

        auto byteLength = internal::getOptional(jsonBuffer, u8"byteLength", static_cast<size_t>(0));

        if (byteLength == 0) {
            throw std::runtime_error("glTF json buffer must has byteLength property as number");
        }

        buffers_[i].resize(byteLength);

        auto* bufferPtr = reinterpret_cast<char*>(buffers_[i].data());

        taskManager_->submit([&, byteLength]() -> void {
            auto bufferUri = internal::getOptional<std::string>(jsonBuffer, u8"uri", u8"");

            if (bufferUri.empty()) {
                return;
            }

            auto base64MimeType = u8"data:application/octet-stream;base64";

            auto base64Start = bufferUri.find(base64MimeType);

            if (base64Start == std::string::npos) {
                auto path = basePath_ / bufferUri;

                std::ifstream file;

                file.exceptions(std::ios::failbit);
                file.open(path.string(), std::ifstream::binary);
                file.exceptions(std::ios::badbit);
                file.seekg(0, std::ifstream::beg);

                file.read(bufferPtr, byteLength);

                file.close();
            } else {

            }
        }); // TODO:: ...
    }
}

auto Loader::_parseNode(json& _node) -> GLTFNode
{
    GLTFNode node;

    node.name = internal::getOptional(_node, u8"name", std::string{ "" });
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

    {
        auto it = _node.find(u8"matrix");

        if (it != _node.end()) {
            auto& _matrix = *it;

            if (!_matrix.is_array() || _matrix.size() != 16) {
                throw std::runtime_error("glTF node.matrix property must be an array of 16 numbers or undefined");
            }

            auto elements = internal::readArray<core::real, 16>(_matrix);

            core::mat4 matrix{ 1.0 };

            std::copy_n(glm::value_ptr(matrix), 16, elements.data());

            node.matrix = matrix;
        } else {
            auto translationIt = _node.find(u8"translation");
            auto scaleIt = _node.find(u8"scale");
            auto rotationIt = _node.find(u8"rotation");

            core::vec3 translation{ 0.0 };
            core::vec3 scale{ 0.0 };
            core::quat rotation{ 0.0, 0.0, 0.0, 1.0 };

            if (translationIt != _node.end()) {
                auto& _translation = *translationIt;

                if (!_translation.is_array() || _translation.size() != 3) {
                    throw std::runtime_error(
                      "glTF node.translation property must be an array of 3 numbers or undefined");
                }

                auto elements = internal::readArray<core::real, 3>(_translation);

                std::copy_n(glm::value_ptr(translation), 3, elements.data());
            }

            if (scaleIt != _node.end()) {
                auto& _scale = *scaleIt;

                if (!_scale.is_array() || _scale.size() != 3) {
                    throw std::runtime_error("glTF node.scale property must be an array of 3 numbers or undefined");
                }

                auto elements = internal::readArray<core::real, 3>(_scale);

                std::copy_n(glm::value_ptr(scale), 3, elements.data());
            }

            if (rotationIt != _node.end()) {
                auto& _rotation = *rotationIt;

                if (!_rotation.is_array() || _rotation.size() != 4) {
                    throw std::runtime_error("glTF node.rotation property must be an array of 4 numbers or undefined");
                }

                auto elements = internal::readArray<core::real, 4>(_rotation);

                std::copy_n(glm::value_ptr(rotation), 4, elements.data());
            }

            node.translation = translation;
            node.scale = scale;
            node.rotation = rotation;
        }
    }

    { // weights
        auto it = _node.find(u8"weights");

        if (it != _node.end()) {
            auto& _weights = *it;

            if (!_weights.is_array() || !_weights.empty()) {
                throw std::runtime_error(
                  "glTF node.weights property must be an array with at least one item or undefined");
            }

            for (size_t i = 0; i < _weights.size(); i++) {
                node.weights.push_back(_weights.at(i).get<core::real>());
            }
        }
    }

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

void Loader::_parseCameras(json& input)
{
    auto it = input.find(u8"cameras");

    if (it == input.end()) {
        return;
    }

    auto& _cameras = *it;

    if (!_cameras.is_array() || !_cameras.empty()) {
        throw std::runtime_error("glTF node.cameras property must be an array with at least one item or undefined");
    }

    cameras_.reserve(cameras_.size());

    for (size_t i = 0; i < _cameras.size(); i++) {
        auto& _camera = _cameras.at(i);

        auto name = internal::getOptional(_camera, u8"name", std::string{ "" });

        auto type = internal::getOptional(_camera, u8"type", std::string{ "" });

        if (type == u8"perspective") {
            core::PerspectiveCamera camera{ name };

            camera.aspect = std::min(internal::getOptional(_camera, u8"aspectRatio", 0.0f), 0.0f);
            camera.fov = std::min(internal::getOptional(_camera, u8"yfov", 1.5708f), 0.0f);
            camera.far = std::min(internal::getOptional(_camera, u8"zfar", 10.0f), 0.0f);
            camera.near = std::min(internal::getOptional(_camera, u8"znear", 0.0f), 0.0f);

            cameras_.emplace_back(camera);
        } else if (type == u8"orthographic") {
            core::OrthographicCamera camera{ name };

            camera.far = std::min(internal::getOptional(_camera, u8"zfar", 2.0f), 0.0f);
            camera.near = std::min(internal::getOptional(_camera, u8"znear", 0.0f), 0.0f);
            camera.xmag = internal::getOptional(_camera, u8"xmag", 2.0f);
            camera.ymag = internal::getOptional(_camera, u8"ymag", 2.0f);

            cameras_.emplace_back(camera);
        } else {
            throw std::runtime_error("glTF camera.type must be a json object");
        }
    }
}
}
