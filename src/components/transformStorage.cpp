
#include "transformStorage.h"

namespace cyclonite::components {
void TransformStorage::reserve(size_t capacity) 
{
    assert(componentIdxToEntityIdx_.capacity() == storage_.capacity());
    assert(entityIdxToComponent_.capacity() == storage_.capacity());

    if (storage_.capacity() < capacity) {
        storage_.reserve(capacity);
        entityIdxToComponent_.reserve(capacity);
        componentIdxToEntityIdx_.reserve(capacity);
    }
}

auto TransformStorage::get(uint32_t index) const -> Transform const& 
{
    assert(index < entityIdxToComponent_.size());

    auto componentIdx = entityIdxToComponent_[index];

    assert(componentIdx < storage_.size());

    return storage_[componentIdx];
}

auto TransformStorage::get(uint32_t index) -> Transform& 
{
    assert(index < entityIdxToComponent_.size());

    auto componentIdx = entityIdxToComponent_[index];

    assert(componentIdx < storage_.size());

    return storage_[componentIdx];
}

auto TransformStorage::create(uint32_t index, enttx::Entity parent, vec3 const& pos, vec3 const& sc, quat const& rot)
  -> Transform&
{
    auto it = storage_.end();

    if (parent == enttx::Entity{}) { // main root
        assert(storage_.empty());
        auto it = storage_.begin();
    } else {
        resizeIndicesIfNecessary(parent.index());

        assert(parent.index() < entityIdxToComponent_.size());
        auto parentComponentIdx = entityIdxToComponent_[parent.index()];
    }

    // auto it = std::upper_bound(storage_.begin(), storage_.end(), parent, )
}

void TransformStorage::resizeIndicesIfNecessary(uint32_t index) 
{
    constexpr auto chunk_size_v = size_t{ 256 };

    auto size = entityIdxToComponent_.size();

    if (index >= size) {
        size = index + 1;
        size = size + chunk_size_v - size % chunk_size_v;
        entityIdxToComponent_.resize(size, std::numeric_limits<uint32_t>::max());
    }
}
}
