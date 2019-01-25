//
// Created by bantdit on 1/19/19.
//

#ifndef CYCLONITE_GLTFLOADER_H
#define CYCLONITE_GLTFLOADER_H

#include <filesystem>
#include <fstream>
#include <istream>
#include <nlohmann/json.hpp>
#include <optional>
#include <vector>

#include "../../core/sceneManager.h"
#include "../../core/typedefs.h"

namespace cyclonite::loaders::gltf {
class Loader
{
public:
    using json = nlohmann::json;

    Loader();

    template<typename SceneManager>
    auto load(std::istream& stream, SceneManager& sceneManager, std::vector<typename SceneManager::scene_t>& scenes)
      -> std::vector<typename SceneManager::scene_t>&;

    template<typename SceneManager>
    auto load(std::filesystem::path const& path, SceneManager& sceneManager, std::vector<core::Scene>& scenes)
      -> std::vector<core::Scene>&;

    template<typename SceneManager>
    auto load(std::pair<void const*, size_t> buffer, SceneManager& sceneManager, std::vector<core::Scene>& scenes)
      -> std::vector<core::Scene>&;

    template<typename SceneManager>
    auto load(std::istream& stream, SceneManager& sceneManager, std::vector<core::Scene>&& scenes)
      -> std::vector<core::Scene>&&;

    template<typename SceneManager>
    auto load(std::filesystem::path const& path, SceneManager& sceneManager, std::vector<core::Scene>&& scenes)
      -> std::vector<core::Scene>&&;

    template<typename SceneManager>
    auto load(std::pair<void const*, size_t> buffer, SceneManager& sceneManager, std::vector<core::Scene>&& scenes)
      -> std::vector<core::Scene>&&;

private:
    struct GLTFNode
    {
        std::string name;
        std::optional<size_t> camera;
        std::optional<size_t> skin;
        std::optional<size_t> mesh;
        std::vector<size_t> children;
        std::optional<core::mat4> matrix;
        std::optional<core::quat> rotation;
        std::optional<core::vec3> scale;
        std::optional<core::vec3> translation;
        std::vector<core::real> weights;
    };

private:
    void _parseAsset(json& input);

    bool _testVersion(json& asset);

    void _parseScenes(json& input, std::vector<core::Scene>& scenes);

    void _parseNodes(json& input);

private:
    std::filesystem::path basePath_;

    std::vector<GLTFNode> nodes_;
};

template<typename SceneManager>
auto Loader::load(std::istream& stream, SceneManager& sceneManager, std::vector<typename SceneManager::scene_t>& scenes)
  -> std::vector<typename SceneManager::scene_t>&
{
    json input;

    stream >> input;

    if (!input.is_object()) {
        throw std::runtime_error("input data is not glTF json");
    }

    _parseAsset(input);

    if constexpr (SceneManager::system_manager_t::template has_system_for_components_v<>) {
    }

    return scenes;
}

template<typename SceneManager>
auto Loader::load(std::filesystem::path const& path, SceneManager& sceneManager, std::vector<core::Scene>& scenes)
  -> std::vector<core::Scene>&
{
    basePath_ = path.parent_path();

    std::ifstream file;
    file.exceptions(std::ios::failbit);
    file.open(path.string());
    file.exceptions(std::ios::badbit);

    return load(file, sceneManager, scenes);
}

template<typename SceneManager>
auto Loader::load(std::pair<void const*, size_t> buffer, SceneManager& sceneManager, std::vector<core::Scene>& scenes)
  -> std::vector<core::Scene>&
{
    std::stringstream stream;

    auto [data, size] = buffer;

    stream.write(static_cast<char const*>(data), size);

    return load(stream, sceneManager, scenes);
}

template<typename SceneManager>
auto Loader::load(std::istream& stream, SceneManager& sceneManager, std::vector<core::Scene>&& scenes)
  -> std::vector<core::Scene>&&
{
    return std::move(load(stream, sceneManager, scenes));
}

template<typename SceneManager>
auto Loader::load(std::filesystem::path const& path, SceneManager& sceneManager, std::vector<core::Scene>&& scenes)
  -> std::vector<core::Scene>&&
{
    return std::move(load(path, sceneManager, scenes));
}

template<typename SceneManager>
auto Loader::load(std::pair<void const*, size_t> buffer, SceneManager& sceneManager, std::vector<core::Scene>&& scenes)
  -> std::vector<core::Scene>&&
{
    return std::move(load(buffer, sceneManager, scenes));
}

void Loader::_parseScenes(json& input, std::vector<core::Scene>& scenes)
{
    auto it = input.find("scenes");

    if (it == input.end()) {
        return;
    }

    auto& _scenes = (*it);

    if (!_scenes.is_array() || scenes.empty()) {
        throw std::runtime_error("glTF scenes property should be an array with at least one item or undefined");
    }
}
}

#endif // CYCLONITE_GLTFLOADER_H
