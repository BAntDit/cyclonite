//
// Created by bantdit on 1/19/19.
//

#ifndef CYCLONITE_GLTFLOADER_H
#define CYCLONITE_GLTFLOADER_H

#include <filesystem>
#include <fstream>
#include <istream>
#include <nlohmann/json.hpp>
#include <vector>

namespace cyclonite::loaders::gltf {
class Loader
{
public:
    using json = nlohmann::json;

    Loader();

    template<typename Scene>
    auto load(std::istream& stream, std::vector<Scene>& scenes) -> std::vector<Scene>&;

    template<typename Scene>
    auto load(std::filesystem::path const& path, std::vector<Scene>& scenes) -> std::vector<Scene>&;

    template<typename Scene>
    auto load(std::pair<void const*, size_t> buffer, std::vector<Scene>& scenes) -> std::vector<Scene>&;

    template<typename Scene>
    auto load(std::istream& stream, std::vector<Scene>&& scenes = {}) -> std::vector<Scene>&&;

    template<typename Scene>
    auto load(std::filesystem::path const& path, std::vector<Scene>&& scenes = {}) -> std::vector<Scene>&&;

    template<typename Scene>
    auto load(std::pair<void const*, size_t> buffer, std::vector<Scene>&& scenes = {}) -> std::vector<Scene>&&;

private:
    void _parseAsset(json& input);

    bool _testVersion(json& asset);

    template<typename Scene>
    void _parseScenes(json& input, std::vector<Scene>& scenes);

    std::filesystem::path basePath_;
};

template<typename Scene>
auto Loader::load(std::istream& stream, std::vector<Scene>& scenes) -> std::vector<Scene>&
{
    json input;

    stream >> input;

    if (!input.is_object()) {
        throw std::runtime_error("input data is not glTF json");
    }

    _parseAsset(input);

    return scenes;
}

template<typename Scene>
auto Loader::load(std::filesystem::path const& path, std::vector<Scene>& scenes) -> std::vector<Scene>&
{
    basePath_ = path.parent_path();

    std::ifstream file;
    file.exceptions(std::ios::failbit);
    file.open(path.string());
    file.exceptions(std::ios::badbit);

    return load(file, scenes);
}

template<typename Scene>
auto Loader::load(std::pair<void const*, size_t> buffer, std::vector<Scene>& scenes) -> std::vector<Scene>&
{
    std::stringstream stream;

    auto [data, size] = buffer;

    stream.write(static_cast<char const*>(data), size);

    return load(stream, scenes);
}

template<typename Scene>
auto Loader::load(std::istream& stream, std::vector<Scene>&& scenes) -> std::vector<Scene>&&
{
    return std::move(load(stream, scenes));
}

template<typename Scene>
auto Loader::load(std::filesystem::path const& path, std::vector<Scene>&& scenes) -> std::vector<Scene>&&
{
    return std::move(load(path, scenes));
}

template<typename Scene>
auto Loader::load(std::pair<void const*, size_t> buffer, std::vector<Scene>&& scenes) -> std::vector<Scene>&&
{
    return std::move(load(buffer, scenes));
}

template<typename Scene>
void Loader::_parseScenes(json& input, std::vector<Scene>& scenes)
{
    // TODO:: ...
}
}

#endif // CYCLONITE_GLTFLOADER_H
