
#ifndef CYCLONITE_GFX_GEOMETRY_INDICES_MANAGER
#define CYCLONITE_GFX_GEOMETRY_INDICES_MANAGER

#include "gfx/common.h"
#include "core/arena.h"
#include "core/resourceSharedRef.h"
#include <mutex>

namespace cyclonite::gfx 
{
class GeometryIndicesManager;

class Device;

namespace internal {
class IndexArena;
}

class IndicesAllocation
{
    friend class internal::IndexArena;

public:
    IndicesAllocation() = default;

    IndicesAllocation(IndicesAllocation const&) = delete;
    
    IndicesAllocation(IndicesAllocation&&) = default;

    ~IndicesAllocation();

    auto operator=(IndicesAllocation const&) -> IndicesAllocation& = delete;
    
    auto operator=(IndicesAllocation&&) -> IndicesAllocation& = default;

    [[nodiscard]] auto indexBuffer() const -> core::ResourceSharedRef const& { return indexBufferRef_; }

    [[nodiscard]] auto firstIndex() const -> uint32_t { return firstIndex_; }

    [[nodiscard]] auto indexCount() const -> uint32_t { return indexCount_; } 
    
    [[nodiscard]] auto indexType() const -> gfx::IndexType { return indexType_; } 
    
    [[nodiscard]] auto lock() -> void*; 
    
    void unlock(); 

private:
    std::mutex lock_;
    internal::IndexArena* arena_;
    core::ResourceSharedRef indexBufferRef_;
    core::ResourceSharedRef stagingBufferRef_;
    uint32_t firstIndex_;
    uint32_t indexCount_;
    gfx::IndexType indexType_;
};

namespace internal {
class IndexArena: protected core::Arena
{
friend class GeometryIndicesManager;

private:
    IndexArena(GeometryIndicesManager& geometryIndicesManager, size_t capacity, gfx::IndexType indexType);

public:
    [[nodiscard]] auto indicesManager() const -> GeometryIndicesManager const& { return *indicesManager_; }
    
    [[nodiscard]] auto indicesManager() -> GeometryIndicesManager& { return *indicesManager_; }
    
    [[nodiscard]] auto alloc(uint32_t indexCount) -> IndicesAllocation;
    
    void free(uint32_t firstIndex, uint32_t indexCount);

    using Arena::freeAll;

    using Arena::capacity;

private:
    GeometryIndicesManager* indicesManager_;
    core::ResourceSharedRef indexBuffer_;
    gfx::IndexType indexType_;
};
}

class GeometryIndicesManager
{
public:
    [[nodiscard]] auto device() const -> gfx::Device const& { return deviceRef_.template as<gfx::Device>(); }

    [[nodiscard]] auto device() -> gfx::Device& { return deviceRef_.template as<gfx::Device>(); }

private:
    core::ResourceSharedRef deviceRef_;
	// TODO:: arena
};
}

#endif // CYCLONITE_GFX_GEOMETRY_INDICES_MANAGER
