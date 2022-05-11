//
// Created by bantdit on 10/26/19.
//

#ifndef CYCLONITE_GLTF_LOADER_H
#define CYCLONITE_GLTF_LOADER_H

#include <filesystem>
#include <fstream>
#include <istream>
#include <nlohmann/json.hpp>
#include <optional>

#include "../../arena.h"
#include "../../multithreading/taskManager.h"
#include "../../typedefs.h"

namespace cyclonite::loaders::gltf {
class Loader
{
public:
    using json = nlohmann::json;

    Loader(multithreading::TaskManager& taskManager);

    template<typename Memory>
    void load(std::filesystem::path const& path, Arena<Memory>& vertexStaging, Arena<Memory>& indexStaging);

    template<typename Memory>
    void load(std::pair<void const*, size_t> buffer, Arena<Memory>& vertexStaging, Arena<Memory>& indexStaging);

    template<typename Memory>
    void load(std::istream& stream, Arena<Memory>& vertexStaging, Arena<Memory>& indexStaging);

private:
    struct GLTFNode
    {
        std::string name;
        size_t camera;
        size_t skin;
        size_t mesh;
        std::vector<size_t> children;
        std::optional<mat4> matrix;
        std::optional<quat> rotation;
        std::optional<vec3> scale;
        std::optional<vec3> translation;
        std::vector<real> weights;
    };

    void _parseAsset(json& input);

    void _readBuffers(json& input);

    multithreading::TaskManager* taskManager_;
    std::filesystem::path basePath_;
    std::vector<std::vector<std::byte>> buffers_;
};

template<typename Memory>
void Loader::load(std::filesystem::path const& path, Arena<Memory>& vertexStaging, Arena<Memory>& indexStaging)
{
    basePath_ = path.parent_path();

    std::ifstream file;
    file.exceptions(std::ios::failbit);
    file.open(path.string());
    file.exceptions(std::ios::badbit);

    load(file, vertexStaging, indexStaging);
}

template<typename Memory>
void Loader::load(std::pair<void const*, size_t> buffer, Arena<Memory>& vertexStaging, Arena<Memory>& indexStaging)
{
    std::stringstream stream;

    auto [data, size] = buffer;

    stream.write(static_cast<char const*>(data), size);

    load(stream, vertexStaging, indexStaging);
}

template<typename Memory>
void Loader::load(std::istream& stream, Arena<Memory>& vertexStaging, Arena<Memory>& indexStaging)
{
    json input;

    stream >> input;

    if (!input.is_object()) {
        throw std::runtime_error("input data is not glTF json");
    }

    _parseAsset(input);

    _readBuffers(input);
}
}

#endif // CYCLONITE_GLTF_LOADER_H
