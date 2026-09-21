//
// Created by anton on 8/5/26.
//

#ifndef CYCLONITE_ENTTX_COMPONENT_STORAGE_H
#define CYCLONITE_ENTTX_COMPONENT_STORAGE_H

#include "componentStorageConcept.h"
#include <cassert>
#include <limits>
#include <memory>
#include <vector>

#if defined(max)
#undef max
#endif
#if defined(min)
#undef min
#endif

namespace cyclonite::enttx {
template<size_t CHUNK_SIZE, size_t INITIAL_CHUNK_COUNT, ComponentConcept ComponentType>
class ComponentStorage
{
public:
    using component_type = ComponentType;

    using storage_type = std::vector<component_type>;

    ComponentStorage();

    [[nodiscard]] auto get(uint32_t index) const -> ComponentType const&;

    [[nodiscard]] auto get(uint32_t index) -> ComponentType&;

    [[nodiscard]] auto capacity() const -> size_t { return storage_.capacity(); }

    [[nodiscard]] auto size() const -> size_t { return storage_.size(); }

    template<typename... Args>
    auto create(uint32_t index, Args&&... args) -> ComponentType&;

    void destroy(uint32_t index);

    [[nodiscard]] auto begin() const -> std::vector<ComponentType>::const_iterator { return storage_.cbegin(); }

    [[nodiscard]] auto end() const -> std::vector<ComponentType>::const_iterator { return storage_.cend(); }

    [[nodiscard]] auto begin() -> std::vector<ComponentType>::iterator { return storage_.begin(); }

    [[nodiscard]] auto end() -> std::vector<ComponentType>::iterator { return storage_.end(); }

    [[nodiscard]] auto getFirstEntityIndex() const -> uint32_t
    {
        return componentIdxToEntityIdx_.empty() ? std::numeric_limits<uint32_t>::max()
                                                : *componentIdxToEntityIdx_.cbegin();
    }

    [[nodiscard]] auto getLastEntityIndex() const -> uint32_t
    {
        return componentIdxToEntityIdx_.empty() ? std::numeric_limits<uint32_t>::max()
                                                : *componentIdxToEntityIdx_.crbegin();
    }

    [[nodiscard]] auto getEntityIndex(uint32_t componentIdx) const -> uint32_t
    {
        return componentIdx < componentIdxToEntityIdx_.size() ? componentIdxToEntityIdx_[componentIdx]
                                                              : std::numeric_limits<uint32_t>::max();
    }

private:
    void resizeIndicesIfNecessary(uint32_t index);

    void reserveStoreIfNecessary(uint32_t componentIdx);

    std::vector<uint32_t> entityIdxToComponent_; // maybe better use unordered map
    std::vector<uint32_t> componentIdxToEntityIdx_;
    std::vector<component_type> storage_;
};
// quick test:
static_assert(ComponentStorageConcept<ComponentStorage<1, 1, uint32_t>>);

template<size_t CHUNK_SIZE, size_t INITIAL_CHUNK_COUNT, ComponentConcept ComponentType>
ComponentStorage<CHUNK_SIZE, INITIAL_CHUNK_COUNT, ComponentType>::ComponentStorage()
  : entityIdxToComponent_(CHUNK_SIZE * INITIAL_CHUNK_COUNT, std::numeric_limits<uint32_t>::max())
  , componentIdxToEntityIdx_{}
  , storage_{}
{
    storage_.reserve(CHUNK_SIZE * INITIAL_CHUNK_COUNT);
    componentIdxToEntityIdx_.reserve(CHUNK_SIZE * INITIAL_CHUNK_COUNT);
}

template<size_t CHUNK_SIZE, size_t INITIAL_CHUNK_COUNT, ComponentConcept ComponentType>
auto ComponentStorage<CHUNK_SIZE, INITIAL_CHUNK_COUNT, ComponentType>::get(uint32_t index) const -> ComponentType const&
{
    assert(index < entityIdxToComponent_.size());

    auto componentIdx = entityIdxToComponent_[index];

    assert(componentIdx < storage_.size());

    return storage_[componentIdx];
}

template<size_t CHUNK_SIZE, size_t INITIAL_CHUNK_COUNT, ComponentConcept ComponentType>
auto ComponentStorage<CHUNK_SIZE, INITIAL_CHUNK_COUNT, ComponentType>::get(uint32_t index) -> ComponentType&
{
    return const_cast<ComponentType&>(std::as_const(*this).get(index));
}

template<size_t CHUNK_SIZE, size_t INITIAL_CHUNK_COUNT, ComponentConcept ComponentType>
template<typename... Args>
auto ComponentStorage<CHUNK_SIZE, INITIAL_CHUNK_COUNT, ComponentType>::create(uint32_t index, Args&&... args)
  -> ComponentType&
{
    auto it = storage_.end();

    if (componentIdxToEntityIdx_.empty()) {
        auto componentIdx = uint32_t{ 0 };

        resizeIndicesIfNecessary(index);

        entityIdxToComponent_[index] = componentIdx;

        reserveStoreIfNecessary(componentIdx);

        it = storage_.emplace(storage_.cbegin(), std::forward<Args>(args)...);
        componentIdxToEntityIdx_.emplace(componentIdxToEntityIdx_.cbegin(), index);
    } else if (auto maxValidEntityIdx = componentIdxToEntityIdx_.back(); index > maxValidEntityIdx) {
        auto componentIdx = entityIdxToComponent_[maxValidEntityIdx];

        resizeIndicesIfNecessary(index);

        entityIdxToComponent_[index] = ++componentIdx;

        reserveStoreIfNecessary(componentIdx);

        it = storage_.emplace(std::next(storage_.cbegin(), componentIdx), std::forward<Args>(args)...);
        componentIdxToEntityIdx_.emplace(std::next(componentIdxToEntityIdx_.cbegin(), componentIdx), index);
    } else {
        auto maxValidEntityIdx = componentIdxToEntityIdx_.back();

        auto componentIdx = std::numeric_limits<uint32_t>::max();

        // increment all indices after the element we're going to place component in
        for (auto componentIdxIt = std::next(entityIdxToComponent_.begin(), index);
             componentIdxIt != std::next(entityIdxToComponent_.begin(), componentIdxToEntityIdx_.crbegin() + 1);
             ++componentIdxIt) {

            if (*componentIdxIt == std::numeric_limits<uint32_t>::max())
                continue; // skips empty

            if (componentIdx == std::numeric_limits<uint32_t>::max()) {
                componentIdx = *componentIdxIt; // index to place before
            }

            (*componentIdxIt)++;
        }
        assert(componentIdx < storage_.size());

        entityIdxToComponent_[index] = componentIdx;

        it = storage_.emplace(std::next(storage_.cbegin(), componentIdx), std::forward<Args>(args)...);

        componentIdxToEntityIdx_.emplace(std::next(componentIdxToEntityIdx_.cbegin(), componentIdx), index);
    }

    assert(it != storage_.end());
    return *it;
}

template<size_t CHUNK_SIZE, size_t INITIAL_CHUNK_COUNT, ComponentConcept ComponentType>
void ComponentStorage<CHUNK_SIZE, INITIAL_CHUNK_COUNT, ComponentType>::destroy(uint32_t index)
{
    auto componentIdx = entityIdxToComponent_[index];

    storage_.erase(std::next(storage_.cbegin(), componentIdx));
    componentIdxToEntityIdx_.erase(std::next(componentIdxToEntityIdx_.cbegin(), componentIdx));

    entityIdxToComponent_[index] = std::numeric_limits<uint32_t>::max();

    for (auto it = std::next(entityIdxToComponent_.begin(), index);
         it != std::next(entityIdxToComponent_.begin(), componentIdxToEntityIdx_.crbegin() + 1);
         it++) {
        if (*it == std::numeric_limits<uint32_t>::max())
            continue;

        (*it)--;
    }
}

template<size_t CHUNK_SIZE, size_t INITIAL_CHUNK_COUNT, ComponentConcept ComponentType>
void ComponentStorage<CHUNK_SIZE, INITIAL_CHUNK_COUNT, ComponentType>::resizeIndicesIfNecessary(uint32_t index)
{
    auto size = entityIdxToComponent_.size();

    if (index >= size) {
        size = index + 1;
        size = size + CHUNK_SIZE - size % CHUNK_SIZE;
        entityIdxToComponent_.resize(size, std::numeric_limits<uint32_t>::max());
    }
}

template<size_t CHUNK_SIZE, size_t INITIAL_CHUNK_COUNT, ComponentConcept ComponentType>
void ComponentStorage<CHUNK_SIZE, INITIAL_CHUNK_COUNT, ComponentType>::reserveStoreIfNecessary(uint32_t componentIdx)
{
    auto capacity = storage_.capacity();

    if (componentIdx >= capacity) {
        capacity = componentIdx + 1;
        capacity = capacity + CHUNK_SIZE - capacity % CHUNK_SIZE;

        storage_.reserve(capacity);
        componentIdxToEntityIdx_.reserve(capacity);
    }
}
}

#endif // CYCLONITE_ENTTX_COMPONENT_STORAGE_H