//
// Created by anton on 12/2/21.
//

#ifndef CYCLONITE_MESHSTORAGE_H
#define CYCLONITE_MESHSTORAGE_H

#include "mesh.h"
#include <enttx/enttx.h>
#include <set>

namespace cyclonite::components {
template<size_t MAX_SUBMESH_COUNT>
class MeshStorage : public enttx::BaseComponentStorage<MeshStorage<MAX_SUBMESH_COUNT>, Mesh>
{
public:
    constexpr static size_t maxSubMeshCount = MAX_SUBMESH_COUNT;

    MeshStorage();

    [[nodiscard]] auto get(uint32_t index) const -> Mesh const& { return meshes_[index]; }

    auto get(uint32_t index) -> Mesh& { return meshes_[index]; }

    auto create(uint32_t index, uint16_t subMeshCount) -> Mesh&;

    void destroy(uint32_t index);

    [[nodiscard]] auto capacity() const -> size_t { return meshes_.capacity(); }

    [[nodiscard]] auto size() const -> size_t { return meshes_.size(); }

    auto begin() const { return meshes_.cbegin(); }

    auto end() const { return meshes_.cend(); }

    auto begin() { return meshes_.begin(); }

    auto end() { return meshes_.end(); }

private:
    std::array<components::SubMesh, maxSubMeshCount> store_;
    std::array<Mesh, maxSubMeshCount> meshes_;
    std::set<std::pair<size_t, size_t>, decltype([](auto a, auto b) -> bool { return a.second < b.second; })>
      freeRanges_;
    std::vector<uint32_t> freeIndices_; // mesh indices
    std::vector<uint32_t> indices_;
};

template<size_t MAX_SUBMESH_COUNT>
MeshStorage<MAX_SUBMESH_COUNT>::MeshStorage()
  : store_{}
  , meshes_{}
  , freeRanges_{}
  , freeIndices_{}
  , indices_(MAX_SUBMESH_COUNT, std::numeric_limits<uint32_t>::max())
{
    std::fill(store_.begin(), store_.end(), SubMesh{});

    freeRanges_.insert(std::make_pair(size_t{ 0 }, store_.size()));

    freeIndices_.reserve(MAX_SUBMESH_COUNT);
    freeIndices_.emplace_back(0);
}

template<size_t MAX_SUBMESH_COUNT>
auto MeshStorage<MAX_SUBMESH_COUNT>::create(uint32_t index, uint16_t subMeshCount) -> Mesh&
{
    auto it =
      std::lower_bound(freeRanges_.cbegin(), freeRanges_.cend(), subMeshCount, [](auto range, auto value) -> bool {
          auto [offset, count] = range;
          (void)offset;

          return count < value;
      });

    if (it == freeRanges_.cend()) {
        throw std::runtime_error("mesh storage has no longer memory to place new mesh");
    }

    auto [rangeOffset, rangeCount] = (*it);

    assert(rangeCount >= subMeshCount);

    if (rangeCount > subMeshCount) {
        auto newCount = rangeCount - subMeshCount;
        auto newOffset = rangeOffset + subMeshCount;

        freeRanges_.insert(std::make_pair(newOffset, newCount));
    }

    freeRanges_.erase(it);

    auto meshIndex = std::numeric_limits<uint32_t>::max();

    if (freeIndices_.size() > 1) {
        meshIndex = freeIndices_.back();
        freeIndices_.pop_back();
    } else {
        assert(!freeIndices_.empty());

        auto& idx = freeIndices_[0];
        meshIndex = idx++;
    }

    if (meshIndex >= meshes_.size()) {
        throw std::runtime_error("mesh storage has no longer memory to place new mesh");
    }

    if (index >= indices_.size()) {
        indices_.resize(indices_.size() * 2, std::numeric_limits<uint32_t>::max());
    }

    assert(indices_[index] == std::numeric_limits<uint32_t>::max());

    meshes_[meshIndex] = Mesh{ store_.data() + rangeOffset, subMeshCount };
    indices_[index] = meshIndex;

    return meshes_[meshIndex];
}

template<size_t MAX_SUBMESH_COUNT>
void MeshStorage<MAX_SUBMESH_COUNT>::destroy(uint32_t index)
{
    assert(indices_[index] != std::numeric_limits<uint32_t>::max());

    auto meshIndex = indices_[index];
    auto& mesh = meshes_[meshIndex];

    auto [ptr, count] = mesh.getSubMeshSetMemory();
    auto offset = ptr - store_.data();

    assert(offset > 0);

    auto freeOffset = offset;
    auto freeCount = count;

    // merge prev if possible
    {
        auto prevIt = std::find_if(freeRanges_.cbegin(), freeRanges_.cend(), [=](auto range) -> bool {
            auto [rangeOffset, rangeCount] = range;
            return (static_cast<size_t>(offset) == (rangeOffset + rangeCount));
        });

        if (prevIt != freeRanges_.cend()) {
            auto [rangeOffset, rangeCount] = (*prevIt);
            freeOffset = rangeOffset;
            freeCount = freeCount + rangeCount;

            freeRanges_.erase(prevIt);
        }
    }

    // merge next if possible
    {
        auto nextIt =
          std::find_if(freeRanges_.cbegin(), freeRanges_.cend(), [offset, count = count](auto range) -> bool {
              auto [rangeOffset, rangeCount] = range;
              (void)rangeCount;

              return (rangeOffset == (offset + count));
          });

        if (nextIt != freeRanges_.cend()) {
            auto [rangeOffset, rangeCount] = (*nextIt);
            freeCount = freeCount + rangeCount;

            freeRanges_.erase(nextIt);
        }
    }

    freeRanges_.insert(std::make_pair(freeOffset, freeCount));
    freeIndices_.push_back(meshIndex);

    indices_[index] = std::numeric_limits<uint32_t>::max();
}
}

#endif // CYCLONITE_MESHSTORAGE_H
