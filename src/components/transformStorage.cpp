
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

auto TransformStorage::create(uint32_t index, enttx::Entity parent) -> Transform&
{
    auto it = storage_.end();
    auto level = uint32_t{ 0 };

    if (parent == enttx::Entity{}) { // main root
        assert(storage_.empty());
        it = storage_.begin();
    } else {
        assert(parent.index() < entityIdxToComponent_.size());
        auto parentComponentIdx = entityIdxToComponent_[parent.index()];

        assert(parentComponentIdx < storage_.size());
        auto& parentTransform = storage_[parentComponentIdx];

        level = parentTransform.level + 1;

        it = std::upper_bound(
          storage_.begin(), storage_.end(), std::pair{ level, parent }, [](auto&& lhs, auto const& rhs) -> bool {
              auto&& [level, parent] = lhs;
              return (level < rhs.level) || (level == rhs.level && parent.index() < rhs.parent.index());
          });
    }

    auto componentIdx = std::distance(storage_.begin(), it);

    reserveStoreIfNecessary(componentIdx);

    auto newIt = storage_.insert(std::next(storage_.begin(), componentIdx), Transform{});
    assert(componentIdx == std::distance(storage_.begin(), newIt));

    auto compIdxIt = componentIdxToEntityIdx_.insert(std::next(componentIdxToEntityIdx_.begin(), componentIdx), index);
    assert(componentIdx == std::distance(componentIdxToEntityIdx_.begin(), compIdxIt));

    resizeIndicesIfNecessary(index);

    entityIdxToComponent_[index] = componentIdx;

    assert(static_cast<size_t>(componentIdx) < componentIdxToEntityIdx_.size());

    for (auto nextComponentIdx = static_cast<size_t>(componentIdx + 1);
         nextComponentIdx < componentIdxToEntityIdx_.size();
         ++nextComponentIdx) {
        auto entityIdx = componentIdxToEntityIdx_[nextComponentIdx];

        entityIdxToComponent_[entityIdx]++;
    }

    auto& transform = *newIt;

    transform.parent = parent;
    transform.level = level;
    transform.state = Transform::State::UPDATE_NOTHING;

    return transform;
}

auto TransformStorage::create(uint32_t index, enttx::Entity parent, vec3 const& position, vec3 const& scale, quat const& rotation)
      -> Transform&
{
    auto& transform = create(index, parent);

    transform.position = position;
    transform.scale = scale;
    transform.orientation = rotation;
    transform.state = Transform::State::UPDATE_LOCAL;

    return transform;
}

auto TransformStorage::create(uint32_t index, enttx::Entity parent, mat4 const& matrix) -> Transform&
{
    auto& transform = create(index, parent);

    transform.matrix = matrix;
    transform.state = Transform::State::UPDATE_COMPONENTS;

    return transform;
}

void TransformStorage::resizeIndicesIfNecessary(uint32_t index)
{
    auto size = entityIdxToComponent_.size();

    if (index >= size) {
        size = index + 1;
        size = size + chunk_size_v - size % chunk_size_v;
        entityIdxToComponent_.resize(size, std::numeric_limits<uint32_t>::max());
    }
}

void TransformStorage::destroy(uint32_t index)
{
    // TODO::
}

void TransformStorage::reserveStoreIfNecessary(uint32_t componentIdx)
{
    auto capacity = storage_.capacity();

    if (componentIdx >= capacity) {
        capacity = componentIdx + 1;
        capacity = capacity + chunk_size_v - capacity % chunk_size_v;

        storage_.reserve(capacity);
        componentIdxToEntityIdx_.reserve(capacity);
    }
}
}
