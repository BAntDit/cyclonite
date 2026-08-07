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

    [[nodiscard]] auto getFirstEntityIndex() const -> uint32_t;

    [[nodiscard]] auto getNextEntityIndex(uint32_t current) const -> uint32_t;

private:
    void resizeIndicesIfNecessary(uint32_t index);

    void reserveStoreIfNecessary(uint32_t componentIdx);

    std::vector<uint32_t> indices_;
    std::vector<component_type> storage_;
    uint32_t indexToMaxValidComponentIndex_;
};
// quick test:
static_assert(ComponentStorageConcept<ComponentStorage<1, 1, uint32_t>>);

template<size_t CHUNK_SIZE, size_t INITIAL_CHUNK_COUNT, ComponentConcept ComponentType>
ComponentStorage<CHUNK_SIZE, INITIAL_CHUNK_COUNT, ComponentType>::ComponentStorage()
  : indices_(CHUNK_SIZE * INITIAL_CHUNK_COUNT, std::numeric_limits<uint32_t>::max())
  , storage_{}
  , indexToMaxValidComponentIndex_{ std::numeric_limits<uint32_t>::max() }
{
    storage_.reserve(CHUNK_SIZE * INITIAL_CHUNK_COUNT);
}

template<size_t CHUNK_SIZE, size_t INITIAL_CHUNK_COUNT, ComponentConcept ComponentType>
auto ComponentStorage<CHUNK_SIZE, INITIAL_CHUNK_COUNT, ComponentType>::get(uint32_t index) const -> ComponentType const&
{
    assert(index < indices_.size());

    auto componentIdx = indices_[index];

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
auto ComponentStorage<CHUNK_SIZE, INITIAL_CHUNK_COUNT, ComponentType>::create(uint32_t index,
                                                                              Args&&... args) -> ComponentType&
{
    assert(index != indexToMaxValidComponentIndex_ || index >= indices_.size() ||
           indices_[index] == std::numeric_limits<uint32_t>::max());

    auto it = storage_.end();

    if (indexToMaxValidComponentIndex_ == std::numeric_limits<uint32_t>::max()) {
        auto componentIdx = uint32_t{ 0 };

        indexToMaxValidComponentIndex_ = index;

        resizeIndicesIfNecessary(index);

        indices_[index] = componentIdx;

        reserveStoreIfNecessary(componentIdx);

        it = storage_.emplace(storage_.cbegin(), std::forward<Args>(args)...);
    } else if (index > indexToMaxValidComponentIndex_) {
        auto componentIdx = indices_[indexToMaxValidComponentIndex_];

        indexToMaxValidComponentIndex_ = index;

        assert(componentIdx < storage_.size());

        resizeIndicesIfNecessary(index);

        indices_[index] = ++componentIdx;

        reserveStoreIfNecessary(componentIdx);

        it = storage_.emplace(std::next(storage_.cbegin(), componentIdx), std::forward<Args>(args)...);
    } else {
        assert(indices_[indexToMaxValidComponentIndex_] < storage_.size());

        reserveStoreIfNecessary(indices_[indexToMaxValidComponentIndex_] + 1);

        auto componentIdx = std::numeric_limits<uint32_t>::max();

        // increment all indices after the element we're going to place component in
        for (auto componentIdxIt = std::next(indices_.begin(), index);
             componentIdxIt != std::next(indices_.begin(), indexToMaxValidComponentIndex_ + 1);
             ++componentIdxIt) {
            if (*componentIdxIt == std::numeric_limits<uint32_t>::max())
                continue;

            if (componentIdx == std::numeric_limits<uint32_t>::max()) {
                componentIdx = *componentIdxIt;
            }

            (*componentIdxIt)++;
        }

        assert(componentIdx < storage_.size());

        indices_[index] = componentIdx;

        it = storage_.emplace(std::next(storage_.cbegin(), componentIdx), std::forward<Args>(args)...);
    }

    assert(it != storage_.end());

    return *it;
}

template<size_t CHUNK_SIZE, size_t INITIAL_CHUNK_COUNT, ComponentConcept ComponentType>
void ComponentStorage<CHUNK_SIZE, INITIAL_CHUNK_COUNT, ComponentType>::destroy(uint32_t index)
{
    assert(index <= indexToMaxValidComponentIndex_);
    assert(indices_[index] != std::numeric_limits<uint32_t>::max());

    auto componentIdx = indices_[index];

    assert(componentIdx < storage_.size());

    storage_.erase(std::next(storage_.cbegin(), componentIdx));

    indices_[index] = std::numeric_limits<uint32_t>::max();

    for (auto it = std::next(indices_.begin(), index);
         it != std::next(indices_.begin(), indexToMaxValidComponentIndex_ + 1);
         it++) {
        if (*it == std::numeric_limits<uint32_t>::max())
            continue;

        (*it)--;
    }

    if (index == indexToMaxValidComponentIndex_) {
        uint32_t validIndex = std::numeric_limits<uint32_t>::max();

        for (uint32_t i = indexToMaxValidComponentIndex_; i != std::numeric_limits<uint32_t>::max(); i--) {
            if (indices_[i] != std::numeric_limits<uint32_t>::max()) {
                validIndex = i;
                break;
            }
        }

        indexToMaxValidComponentIndex_ = validIndex;
    }
}

template<size_t CHUNK_SIZE, size_t INITIAL_CHUNK_COUNT, ComponentConcept ComponentType>
void ComponentStorage<CHUNK_SIZE, INITIAL_CHUNK_COUNT, ComponentType>::resizeIndicesIfNecessary(uint32_t index)
{
    auto size = indices_.size();

    if (index >= size) {
        size = index + 1;
        size = size + CHUNK_SIZE - size % CHUNK_SIZE;
        indices_.resize(size, std::numeric_limits<uint32_t>::max());
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
    }
}

template<size_t CHUNK_SIZE, size_t INITIAL_CHUNK_COUNT, ComponentConcept ComponentType>
auto ComponentStorage<CHUNK_SIZE, INITIAL_CHUNK_COUNT, ComponentType>::getNextEntityIndex(uint32_t current) const
  -> uint32_t
{
    auto index = std::numeric_limits<uint32_t>::max();
    for (auto i = ++current; i <= indexToMaxValidComponentIndex_; i++) {
        if (indices_[i] != std::numeric_limits<uint32_t>::max()) {
            index = i;
            break;
        }
    }
    return index;
}

template<size_t CHUNK_SIZE, size_t INITIAL_CHUNK_COUNT, ComponentConcept ComponentType>
auto ComponentStorage<CHUNK_SIZE, INITIAL_CHUNK_COUNT, ComponentType>::getFirstEntityIndex() const -> uint32_t
{
    auto index = std::numeric_limits<uint32_t>::max();
    for (auto i = uint32_t{ 0 }; i <= indexToMaxValidComponentIndex_; i++) {
        if (indices_[i] != std::numeric_limits<uint32_t>::max()) {
            index = i;
            break;
        }
    }
    return index;
}
}

#endif // CYCLONITE_ENTTX_COMPONENT_STORAGE_H