//
// Created by bantdit on 1/13/20.
//

#ifndef CYCLONITE_TRANSFORMSTORAGE_H
#define CYCLONITE_TRANSFORMSTORAGE_H

#include "transform.h"
#include <enttx/enttx.h>

namespace cyclonite::components {
template<size_t CHUNK_SIZE, size_t INITIAL_CHUNK_COUNT>
class TransformStorage
  : public enttx::BaseComponentStorage<TransformStorage<CHUNK_SIZE, INITIAL_CHUNK_COUNT>, Transform>
{
public:
    constexpr static size_t chunkSize = CHUNK_SIZE;

    constexpr static size_t initialChunkCount = INITIAL_CHUNK_COUNT;

    TransformStorage();

    auto get(uint32_t index) const -> Transform const&;

    auto get(uint32_t index) -> Transform&;

    template<typename... Args>
    auto create(uint32_t index, size_t depth, Args&&... args) -> Transform&;

    void destroy(uint32_t index);

    auto capacity() const -> size_t { return store_.capacity(); }

    auto size() const -> size_t { return store_.size(); }

    auto begin() const { return store_.cbegin(); }

    auto end() const { return store_.cend(); }

    auto begin() { return store_.begin(); }

    auto end() { return store_.end(); }

private:
    void _reserveStoreIfNecessary(size_t pos);

    void _resizeIndicesIfNecessary(uint32_t index);

    std::vector<size_t> indices_;
    std::vector<Transform> store_;

    uint32_t maxValidIndex_;
};

template<size_t CHUNK_SIZE, size_t INITIAL_CHUNK_COUNT>
TransformStorage<CHUNK_SIZE, INITIAL_CHUNK_COUNT>::TransformStorage()
  : indices_(CHUNK_SIZE * INITIAL_CHUNK_COUNT, std::numeric_limits<uint32_t>::max())
  , store_{}
  , maxValidIndex_{ std::numeric_limits<uint32_t>::max() }
{
    store_.reserve(CHUNK_SIZE * INITIAL_CHUNK_COUNT);
}

template<size_t CHUNK_SIZE, size_t INITIAL_CHUNK_COUNT>
auto TransformStorage<CHUNK_SIZE, INITIAL_CHUNK_COUNT>::get(uint32_t index) const -> Transform const&
{
    assert(index < indices_.size());

    auto pos = indices_[index];

    assert(pos < store_.size());

    return store_[pos];
}

template<size_t CHUNK_SIZE, size_t INITIAL_CHUNK_COUNT>
auto TransformStorage<CHUNK_SIZE, INITIAL_CHUNK_COUNT>::get(uint32_t index) -> Transform&
{
    assert(index < indices_.size());

    auto pos = indices_[index];

    assert(pos < store_.size());

    return store_[pos];
}

template<size_t CHUNK_SIZE, size_t INITIAL_CHUNK_COUNT>
template<typename... Args>
auto TransformStorage<CHUNK_SIZE, INITIAL_CHUNK_COUNT>::create(uint32_t index, size_t globalIndex, Args&&... args)
  -> Transform&
{
    assert(index != maxValidIndex_ || index >= indices_.size() ||
           indices_[index] == std::numeric_limits<uint32_t>::max());

    auto it = store_.end();

    auto pos = globalIndex;

    if (maxValidIndex_ == std::numeric_limits<uint32_t>::max() || index > maxValidIndex_)
        maxValidIndex_ = index;

    _resizeIndicesIfNecessary(index);
    _reserveStoreIfNecessary(pos);

    it = store_.insert(std::next(store_.begin(), pos), Transform{ std::forward<Args>(args)... });

    for (auto iit = indices_.begin(); iit != std::next(indices_.begin(), maxValidIndex_ + 1); iit++) {
        if (*iit == std::numeric_limits<uint32_t>::max())
            continue;

        if (*iit >= pos)
            (*iit)++;
    }

    indices_[index] = pos;

    assert(it != store_.end());

    return *it;
}

template<size_t CHUNK_SIZE, size_t INITIAL_CHUNK_COUNT>
void TransformStorage<CHUNK_SIZE, INITIAL_CHUNK_COUNT>::destroy(uint32_t index)
{
    assert(index <= maxValidIndex_);

    assert(indices_[index] != std::numeric_limits<uint32_t>::max());

    auto pos = indices_[index];

    assert(pos < store_.size());

    store_.erase(std::next(store_.cbegin(), pos));

    indices_[index] = std::numeric_limits<uint32_t>::max();

    for (auto it = indices_.begin(); it != std::next(indices_.begin(), maxValidIndex_ + 1); it++) {
        if (*it == std::numeric_limits<uint32_t>::max())
            continue;

        if (*it > pos)
            (*it)--;
    }

    if (index == maxValidIndex_) {
        uint32_t validIndex = std::numeric_limits<uint32_t>::max();

        for (uint32_t i = maxValidIndex_; i != std::numeric_limits<uint32_t>::max(); i--) {
            if (indices_[i] != std::numeric_limits<uint32_t>::max()) {
                validIndex = i;
                break;
            }
        }

        maxValidIndex_ = validIndex;
    }
}

template<size_t CHUNK_SIZE, size_t INITIAL_CHUNK_COUNT>
void TransformStorage<CHUNK_SIZE, INITIAL_CHUNK_COUNT>::_reserveStoreIfNecessary(size_t pos)
{
    auto capacity = store_.capacity();

    if (pos >= capacity) {
        capacity = pos + 1;
        capacity = capacity + CHUNK_SIZE - capacity % CHUNK_SIZE;

        store_.reserve(capacity);
    }
}

template<size_t CHUNK_SIZE, size_t INITIAL_CHUNK_COUNT>
void TransformStorage<CHUNK_SIZE, INITIAL_CHUNK_COUNT>::_resizeIndicesIfNecessary(uint32_t index)
{
    auto size = indices_.size();

    if (index >= size) {
        size = index + 1;
        size = size + CHUNK_SIZE - size % CHUNK_SIZE;

        indices_.resize(size, std::numeric_limits<uint32_t>::max());
    }
}
}

#endif // CYCLONITE_TRANSFORMSTORAGE_H
