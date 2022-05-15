//
// Created by bantdit on 7/17/20.
//

#ifndef CYCLONITE_READER_H
#define CYCLONITE_READER_H

#include <boost/scope_exit.hpp>
#include <cyclonite.h>
#include <filesystem>
#include <fstream>
#include <glm/gtx/matrix_decompose.hpp>
#include <istream>
#include <nlohmann/json.hpp>
#include <regex>
#include <unordered_map>

// dummy gltf reader implementation
namespace examples::viewer::gltf {
using namespace cyclonite;

namespace {
auto _testVersion(nlohmann::json const& asset) -> bool
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
        std::regex re("^(\\d+).(\\d+)($|.\\d+$|(.\\d+.\\d+$))");
        std::smatch matches;

        auto it = asset.find(reinterpret_cast<char const*>(u8"minVersion"));

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
                    return !(supportedMajorVersion == majorVersion && supportedMinorVersion < minorVersion);
                }

                return false;
            }

            throw std::runtime_error("glTF asset has wrong the minVersion format");
        }
    }

    // check version
    {
        auto it = asset.find(reinterpret_cast<char const*>(u8"version"));

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
        std::regex re("^(\\d+)($|.(\\d+))($|.\\d+$|(.\\d+.\\d+$))");
        std::smatch matches;

        if (std::regex_match(version, matches, re)) {
            assert(!matches.empty());

            auto majorVersion = static_cast<uint16_t>(strtol(matches[0].str().c_str(), nullptr, 10));

            return majorVersion >= minimalMajorVersion;
        }

        throw std::runtime_error("glTF asset has wrong the version format");
    }

    std::terminate();
}

template<typename T>
auto _getOptional(nlohmann::json const& json, std::string const& property, T&& defaultValue) -> std::decay_t<T>
{
    assert(json.is_object());

    typename std::decay<T>::type result;

    auto it = json.find(property);

    if (it != json.end()) {
        try {
            result = it.value().get<T>();
        } catch (...) {
            result = std::forward<T>(defaultValue);
        }
    } else {
        result = std::forward<T>(defaultValue);
    }

    return result;
}

auto _getJsonProperty(nlohmann::json const& json, std::string const& property) -> nlohmann::json const&
{
    assert(json.is_object());

    auto it = json.find(property);

    assert(it != json.end());

    return *it;
}

template<typename T, size_t Size>
auto _readArray(nlohmann::json const& array) -> std::array<T, Size>
{
    std::array<T, Size> result{};

    assert(array.is_array() && array.size() >= Size);

    for (size_t i = 0; i < Size; i++) {
        result[i] = array.at(i).get<T>();
    }

    return result;
}
enum class ReaderDataType : uint8_t
{
    NODE_COUNT,
    BUFFER,
    BUFFER_VIEW,
    ACCESSOR,
    VERTEX_INDEX_INSTANCE_COUNT,
    NODE,
    GEOMETRY,
    MESH,
    COUNT,
    MIN_VALUE = NODE_COUNT,
    MAX_VALUE = COUNT
};

template<ReaderDataType DataTypeValue>
using reader_data_type_t = std::integral_constant<ReaderDataType, DataTypeValue>;

template<ReaderDataType DataValue, typename DataType>
inline constexpr auto reader_data_test() -> bool
{
    return std::is_same_v<std::decay_t<DataType>, reader_data_type_t<DataValue>>;
}
}

class Reader
{
public:
    struct BufferView
    {
        size_t bufferIdx;
        size_t byteOffset;
        size_t byteLength;
        size_t byteStride;
    };

    struct Accessor
    {
        size_t bufferViewIdx;
        size_t byteOffset;
        uint32_t componentType;
        bool normalized;
        uint32_t count;
        std::string type;
    };

    struct Primitive
    {
        size_t idxPosition;
        size_t idxNormal;
        size_t idxIndex;
    };

    struct Node
    {
        Node(vec3& p, vec3& s, quat& r)
          : position{ p }
          , scale{ s }
          , rotation{ r }
        {}

        vec3 position;
        vec3 scale;
        quat rotation;
    };

public:
    Reader() = default;

    [[nodiscard]] auto accessors() const -> std::vector<Accessor> const& { return accessors_; }

    [[nodiscard]] auto bufferViews() const -> std::vector<BufferView> const& { return bufferViews_; }

    [[nodiscard]] auto buffers() const -> std::vector<std::vector<std::byte>> const& { return buffers_; }

    [[nodiscard]] auto buffers() -> std::vector<std::vector<std::byte>>& { return buffers_; }

    template<typename F>
    void read(std::pair<void const*, size_t> buffer, F&& f);

    template<typename F>
    void read(std::filesystem::path const& path, F&& f);

    template<typename F>
    void read(std::istream& stream, F&& f);

private:
    using intance_key_t = std::tuple<size_t, size_t, size_t>;

    void _countNode(
      nlohmann::json const& nodes,
      nlohmann::json const& meshes,
      nlohmann::json const& accessors,
      std::unordered_map<intance_key_t, std::tuple<uint32_t, uint32_t, uint32_t>, cyclonite::hash>& instanceCommands,
      size_t nodeIdx);

    template<typename F>
    void _readNode(
      nlohmann::json const& nodes,
      nlohmann::json const& meshes,
      nlohmann::json const& accessors,
      std::unordered_map<intance_key_t, std::tuple<uint32_t, uint32_t, uint32_t>, cyclonite::hash>& instanceCommands,
      size_t parentIdx,
      size_t nodeIdx,
      F&& f);

    std::filesystem::path basePath_;
    std::vector<std::vector<std::byte>> buffers_;
    std::vector<BufferView> bufferViews_;
    std::vector<Accessor> accessors_;

    uint32_t vertexCount_;
    uint32_t indexCount_;
    uint32_t instanceCount_;
};

template<typename F>
void Reader::read(std::filesystem::path const& path, F&& f)
{
    basePath_ = path.parent_path();

    std::ifstream file;
    file.exceptions(std::ios::failbit);
    file.open(path.string());
    file.exceptions(std::ios::badbit);

    read(file, std::forward<F>(f));
}

template<typename F>
void Reader::read(std::pair<void const*, size_t> buffer, F&& f)
{
    std::stringstream stream;

    auto [data, size] = buffer;

    stream.write(static_cast<char const*>(data), size);

    read(stream, std::forward<F>(f));
}

template<typename F>
void Reader::read(std::istream& stream, F&& f)
{
    buffers_.clear();
    bufferViews_.clear();
    accessors_.clear();

    vertexCount_ = 0;
    indexCount_ = 0;

    nlohmann::json input;

    stream >> input;

    if (!input.is_object()) {
        throw std::runtime_error("input data is not glTF json");
    }

    auto it = input.find(reinterpret_cast<char const*>(u8"asset"));

    if (it == input.end()) {
        throw std::runtime_error("glTF json must contain asset object");
    }

    nlohmann::json& asset = (*it);

    if (!asset.is_object()) {
        throw std::runtime_error("glTF json must contain asset object");
    }

    if (!_testVersion(asset)) {
        throw std::runtime_error("asset has unsupported glTF version");
    }

    // buffers:
    {
        auto const& buffers = _getJsonProperty(input, reinterpret_cast<char const*>(u8"buffers"));

        auto bufferCount = buffers.size();

        buffers_.reserve(bufferCount);

        for (size_t i = 0; i < bufferCount; i++) {
            auto const& jsonBuffer = buffers.at(i);

            if (!jsonBuffer.is_object()) {
                throw std::runtime_error("glTF json buffer should be an object");
            }

            auto byteLength =
              _getOptional(jsonBuffer, reinterpret_cast<char const*>(u8"byteLength"), static_cast<size_t>(0));

            if (byteLength == 0) {
                throw std::runtime_error("glTF json buffer must has byteLength property as number");
            }

            auto& buffer = buffers_.emplace_back(byteLength, std::byte{ 0 });

            auto bufferUri = _getOptional<std::string>(jsonBuffer, reinterpret_cast<char const*>("uri"), "");

            auto path = basePath_ / bufferUri;

            auto* bufferPtr = reinterpret_cast<char*>(buffer.data());

            {
                std::ifstream file;

                file.exceptions(std::ios::failbit);
                file.open(path.string(), std::ifstream::binary);
                file.exceptions(std::ios::badbit);
                file.seekg(0, std::ifstream::beg);
                file.read(bufferPtr, byteLength);
                file.close();
            }

            f(reader_data_type_t<ReaderDataType::BUFFER>{}, buffers_[i], i);
        }
    }

    // buffer views:
    {
        auto const& bufferViews = _getJsonProperty(input, reinterpret_cast<char const*>(u8"bufferViews"));

        auto bufferViewCount = bufferViews.size();

        bufferViews_.reserve(bufferViewCount);

        for (size_t i = 0; i < bufferViewCount; i++) {
            auto const& jsonBufferView = bufferViews.at(i);

            auto& bufferView = bufferViews_.emplace_back();

            bufferView.bufferIdx = _getOptional(
              jsonBufferView, reinterpret_cast<char const*>(u8"buffer"), std::numeric_limits<size_t>::max());

            if (bufferView.bufferIdx == std::numeric_limits<size_t>::max()) {
                throw std::runtime_error("buffer view must contains buffer index.");
            }

            bufferView.byteOffset =
              _getOptional(jsonBufferView, reinterpret_cast<char const*>(u8"byteOffset"), size_t{ 0 });
            bufferView.byteLength =
              _getOptional(jsonBufferView, reinterpret_cast<char const*>(u8"byteLength"), size_t{ 0 });

            if (bufferView.byteLength == 0) {
                throw std::runtime_error("buffer view length must be not less then 1");
            }

            bufferView.byteStride =
              _getOptional(jsonBufferView, reinterpret_cast<char const*>(u8"byteStride"), size_t{ 0 });

            if (bufferView.byteStride % 4 || bufferView.byteStride > 252) {
                throw std::runtime_error("buffer view stride has wrong value");
            }

            f(reader_data_type_t<ReaderDataType::BUFFER_VIEW>{}, bufferViews_[i], i);
        }
    }

    // accessors:
    auto const& accessors = _getJsonProperty(input, reinterpret_cast<char const*>(u8"accessors"));

    {
        auto accessorCount = accessors.size();

        accessors_.reserve(accessorCount);

        for (size_t i = 0; i < accessorCount; i++) {
            auto const& jsonAccessor = accessors.at(i);

            auto& accessor = accessors_.emplace_back();

            accessor.bufferViewIdx = _getOptional(
              jsonAccessor, reinterpret_cast<char const*>(u8"bufferView"), std::numeric_limits<size_t>::max());
            accessor.byteOffset =
              _getOptional(jsonAccessor, reinterpret_cast<char const*>(u8"byteOffset"), size_t{ 0 });

            accessor.componentType = _getOptional(
              jsonAccessor, reinterpret_cast<char const*>(u8"componentType"), std::numeric_limits<uint32_t>::max());

            if (accessor.componentType == std::numeric_limits<uint32_t>::max()) {
                throw std::runtime_error("accessor must contain componentType");
            }

            accessor.normalized = _getOptional(jsonAccessor, reinterpret_cast<char const*>(u8"normalized"), false);

            accessor.count = _getOptional(jsonAccessor, reinterpret_cast<char const*>(u8"count"), uint32_t{ 0 });

            if (accessor.count < 1) {
                throw std::runtime_error("accessor count must be not less then 1");
            }

            accessor.type = _getOptional<std::string>(jsonAccessor, reinterpret_cast<char const*>(u8"type"), "");

            if (accessor.type.empty()) {
                throw std::runtime_error("accessor type has wrong value");
            }

            f(reader_data_type_t<ReaderDataType::ACCESSOR>{}, accessors_[i], i);
        }
    }

    using intance_key_t = std::tuple<size_t, size_t, size_t>;

    std::unordered_map<intance_key_t, std::tuple<uint32_t, uint32_t, uint32_t>, cyclonite::hash> instanceCommands = {};

    auto const& scenes = _getJsonProperty(input, reinterpret_cast<char const*>(u8"scenes"));
    auto const& nodes = _getJsonProperty(input, reinterpret_cast<char const*>(u8"nodes"));
    auto defaultSceneIdx = _getOptional(input, reinterpret_cast<char const*>(u8"scene"), static_cast<size_t>(0));
    auto const& meshes = _getJsonProperty(input, reinterpret_cast<char const*>(u8"meshes"));
    auto const& scene = scenes.at(defaultSceneIdx);
    auto const& rootNodes = _getJsonProperty(scene, reinterpret_cast<char const*>(u8"nodes"));

    f(reader_data_type_t<ReaderDataType::NODE_COUNT>{}, static_cast<uint32_t>(nodes.size()));

    for (size_t i = 0, count = rootNodes.size(); i < count; i++) {
        auto idx = rootNodes.at(i).get<size_t>();
        _countNode(nodes, meshes, accessors, instanceCommands, idx);
    }

    f(reader_data_type_t<ReaderDataType::VERTEX_INDEX_INSTANCE_COUNT>{},
      vertexCount_,
      indexCount_,
      instanceCount_,
      static_cast<uint32_t>(instanceCommands.size()));

    for (size_t i = 0, count = rootNodes.size(); i < count; i++) {
        auto idx = rootNodes.at(i).get<size_t>();

        _readNode(
          nodes, meshes, accessors, instanceCommands, std::numeric_limits<size_t>::max(), idx, std::forward<F>(f));
    }
}

template<typename F>
void Reader::_readNode(
  nlohmann::json const& nodes,
  nlohmann::json const& meshes,
  nlohmann::json const& accessors,
  std::unordered_map<intance_key_t, std::tuple<uint32_t, uint32_t, uint32_t>, cyclonite::hash>& instanceCommands,
  size_t parentIdx,
  size_t nodeIdx,
  F&& f)
{
    auto const& node = nodes.at(nodeIdx);

    auto matrixIt = node.find(reinterpret_cast<char const*>(u8"matrix"));

    auto position = vec3{ 0.f };
    auto scale = vec3{ 1.f };
    auto rotation = quat{ 1.f, vec3{ 0.f } };

    if (matrixIt != node.end()) {
        auto matrix = glm::make_mat4(_readArray<real, 16>(*matrixIt).data());
        auto skew = vec3{};
        auto perspective = vec4{};

        glm::decompose(matrix, scale, rotation, position, skew, perspective);
    } else {
        if (auto it = node.find(reinterpret_cast<char const*>(u8"translation")); it != node.end()) {
            position = glm::make_vec3(_readArray<real, 3>(*it).data());
        }

        if (auto it = node.find(reinterpret_cast<char const*>(u8"scale")); it != node.end()) {
            scale = glm::make_vec3(_readArray<real, 3>(*it).data());
        }

        if (auto it = node.find(reinterpret_cast<char const*>(u8"rotation")); it != node.end()) {
            rotation = glm::make_quat(_readArray<real, 4>(*it).data());
        }
    }

    auto idxMesh = _getOptional(node, reinterpret_cast<char const*>(u8"mesh"), std::numeric_limits<size_t>::max());

    std::vector<Primitive> meshPrimitives = {};

    if (idxMesh != std::numeric_limits<size_t>::max()) {
        auto const& mesh = meshes.at(idxMesh);
        auto const& primitives = _getJsonProperty(mesh, reinterpret_cast<char const*>(u8"primitives"));

        meshPrimitives.reserve(primitives.size());

        for (size_t j = 0, primitiveCount = primitives.size(); j < primitiveCount; j++) {
            auto& primitive = primitives.at(j);

            auto idxIndices =
              _getOptional(primitive, reinterpret_cast<char const*>(u8"indices"), std::numeric_limits<size_t>::max());

            if (idxIndices >= accessors.size()) // skip non-indexed geometry
                continue;

            auto attributes = _getJsonProperty(primitive, reinterpret_cast<char const*>(u8"attributes"));

            auto idxPositions =
              _getOptional(attributes, reinterpret_cast<char const*>(u8"POSITION"), std::numeric_limits<size_t>::max());
            auto idxNormals =
              _getOptional(attributes, reinterpret_cast<char const*>(u8"NORMAL"), std::numeric_limits<size_t>::max());

            if (idxPositions >= accessors.size() || idxNormals >= accessors.size())
                continue;

            auto& meshPrimitive = meshPrimitives.emplace_back();

            meshPrimitive.idxPosition = idxPositions;
            meshPrimitive.idxNormal = idxNormals;
            meshPrimitive.idxIndex = idxIndices;

            auto it = instanceCommands.find(std::make_tuple(idxIndices, idxPositions, idxNormals));

            assert(it != instanceCommands.end());

            auto& [key, value] = (*it);

            if (std::get<0>(value)++ == 0) {
                f(reader_data_type_t<ReaderDataType::GEOMETRY>{}, meshPrimitive);
            }
        } // primitives cycle end
    }     // mesh parse end

    f(reader_data_type_t<ReaderDataType::NODE>{}, Node{ position, scale, rotation }, parentIdx, nodeIdx);

    if (auto childrenIt = node.find(reinterpret_cast<char const*>(u8"children")); childrenIt != node.end()) {
        auto const& children = *childrenIt;

        for (size_t i = 0, count = children.size(); i < count; i++) {
            auto idx = children.at(i).get<size_t>();
            _readNode(nodes, meshes, accessors, instanceCommands, nodeIdx, idx, std::forward<F>(f));
        }
    }

    if (!meshPrimitives.empty())
        f(reader_data_type_t<ReaderDataType::MESH>{}, meshPrimitives, nodeIdx);
}
}

#endif // CYCLONITE_READER_H
