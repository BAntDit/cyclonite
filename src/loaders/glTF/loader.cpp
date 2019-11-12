//
// Created by bantdit on 10/26/19.
//

#include "loader.h"
#include "internal/base64Decode.h"
#include "internal/getOptional.h"
#include "internal/testVersion.h"
#include <regex>

namespace cyclonite::loaders::gltf {
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

    if (!internal::testVersion(asset)) {
        throw std::runtime_error("asset has unsupported glTF version");
    }
}

void Loader::_readBuffers(json& input)
{
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

        // auto* bufferPtr = reinterpret_cast<char*>(buffers_[i].data());

        /* futures[i] = taskManager_->submit([&, byteLength]() -> void {
            auto bufferUri = internal::getOptional<std::string>(jsonBuffer, u8"uri", u8"");

            if (bufferUri.empty()) {
                return;
            }

            auto base64MimeType = std::string(u8"data:application/octet-stream;base64");

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
                auto base64Data = bufferUri.substr(base64MimeType.size() + 1); // +1 for comma character

                std::copy_n(base64Decode(base64Data).begin(), byteLength, buffers_[i].begin());
            }
        }); */
    }

    for (auto&& future : futures) {
        future.get();
    }
}
}