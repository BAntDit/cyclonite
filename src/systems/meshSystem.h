//
// Created by bantdit on 2/6/20.
//

#ifndef CYCLONITE_MESHSYSTEM_H
#define CYCLONITE_MESHSYSTEM_H

#include "../gfx/vulkan/vkDevice.h"
#include "components/mesh.h"
#include "components/transform.h"
#include "resources/resourceManager.h"
#include "resources/staging.h"
#include "transformSystem.h"
#include <enttx/enttx.h>
#include <glm/gtc/type_ptr.hpp>
#include <metrix/containers.h>
#include <metrix/enum.h>

namespace cyclonite {
class Root;
}

namespace cyclonite::resources {
class ResourceManager;
}

namespace cyclonite::systems {
class MeshSystem : public enttx::BaseSystem<MeshSystem>
{
public:
    using tag_t = metrix::type_list<components::Mesh>;

    MeshSystem() = default;

    MeshSystem(MeshSystem const&) = delete;

    MeshSystem(MeshSystem&&) = default;

    ~MeshSystem() = default;

    auto operator=(MeshSystem const&) -> MeshSystem& = delete;

    auto operator=(MeshSystem&&) -> MeshSystem& = default;

    auto createGeometry(uint32_t vertexCount, uint32_t indexCount) -> uint64_t;

    template<typename EntityManager, typename Geometries>
    auto createMesh(EntityManager& entityManager, enttx::Entity entity, Geometries&& geometries)
      -> std::enable_if_t<metrix::is_contiguous_v<Geometries> || std::is_same_v<uint64_t, std::decay_t<Geometries>>,
                          components::Mesh&>;

    // TODO:: delete Mesh

    void init(Root& root,
              size_t swapChainLength,
              size_t initialCommandCapacity,
              size_t initialInstanceCapacity,
              size_t initialIndexCapacity,
              size_t initialVertexCapacity);

    template<typename SystemManager, typename EntityManager, size_t STAGE, typename... Args>
    void update(SystemManager& systemManager, EntityManager& entityManager, Args&&... args);

    void requestVertexDeviceBufferUpdate();

private:
    void _init(Root& root,
               size_t swapChainLength,
               size_t initialCommandCapacity,
               size_t initialInstanceCapacity,
               size_t initialIndexCapacity,
               size_t initialVertexCapacity);

    void _addSubMesh(components::SubMesh& subMesh, uint64_t geometryId);

    void _reAllocCommandBuffer(size_t size);

    auto _getDumpCommandIndex() -> size_t;

private:
    bool verticesUpdateRequired_;
};

template<typename EntityManager, typename Geometries>
auto MeshSystem::createMesh(EntityManager& entityManager, enttx::Entity entity, Geometries&& geometries)
  -> std::enable_if_t<metrix::is_contiguous_v<Geometries> || std::is_same_v<uint64_t, std::decay_t<Geometries>>,
                      components::Mesh&>
{
    auto subMeshCount = uint16_t{ 0 };
    auto geometryIdentifiers = std::add_pointer_t<uint64_t>{ nullptr };
    if constexpr (metrix::is_contiguous_v<Geometries>) {
        assert(std::size(geometries) < std::numeric_limits<uint16_t>::max());
        subMeshCount = static_cast<uint16_t>(std::size(geometries));
        geometryIdentifiers = std::data(geometries);
    } else {
        subMeshCount = uint16_t{ 1 };
        geometryIdentifiers = &geometries;
    }

    auto& mesh = entityManager.template assign<components::Mesh>(entity, subMeshCount);
    assert(subMeshCount == mesh.getSubMeshCount());

    for (auto i = uint16_t{ 0 }; i < subMeshCount; i++) {
        _addSubMesh(mesh.getSubMesh(i), *(geometryIdentifiers + i));
    }

    return mesh;
}

template<typename SystemManager, typename EntityManager, size_t STAGE, typename... Args>
void MeshSystem::update(SystemManager& systemManager, EntityManager& entityManager, Args&&... args)
{
    (void)systemManager;

    (void)entityManager;

    ((void)args, ...);
}
}

#endif // CYCLONITE_MESHSYSTEM_H
