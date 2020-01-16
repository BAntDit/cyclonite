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
    TransformStorage();

    auto get(uint32_t index) const -> Transform const&;

    auto get(uint32_t index) -> Transform&;

    template<typename... Args>
    auto create(uint32_t index, Args&&... args) -> Transform&;

    void destroy(uint32_t index);

    auto capacity() const -> size_t { return store_.capacity(); }

    auto size() const -> size_t { return store_.size(); }

    auto begin() const { return store_.cbegin(); }

    auto end() const { return store_.cend(); }

    auto begin() { return store_.begin(); }

    auto end() { return store_.end(); }

private:
    void _reserveStoreIfNecessary(uint32_t pos);

    void _resizeIndicesIfNecessary(uint32_t index);

    std::vector<uint32_t> indices_;
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
auto TransformStorage<CHUNK_SIZE, INITIAL_CHUNK_COUNT>::create(uint32_t index, Args&&... args) -> Transform&
{
    assert(index != maxValidIndex_ || index >= indices_.size() ||
           indices_[index] == std::numeric_limits<uint32_t>::max());

    auto it = store_.end();

    if (maxValidIndex_ == std::numeric_limits<uint32_t>::max()) {
        uint32_t pos = 0;

        maxValidIndex_ = index;

        _resizeIndicesIfNecessary(index);

        indices_[index] = pos;

        _reserveStoreIfNecessary(pos);

        it = store_.emplace(store_.cbegin(), std::forward<Args>(args)...);
    } else if (index > maxValidIndex_) {
        auto pos = indices_[maxValidIndex_];

        maxValidIndex_ = index;

        assert(pos < store_.size());

        _resizeIndicesIfNecessary(index);

        indices_[index] = ++pos;

        _reserveStoreIfNecessary(pos);

        it = store_.emplace(std::next(store_.cbegin(), pos), std::forward<Args>(args)...);
    } else {
        assert(indices_[maxValidIndex_] < store_.size());

        _reserveStoreIfNecessary(indices_[maxValidIndex_] + 1);

        auto pos = std::numeric_limits<uint32_t>::max();

        for (auto it = std::next(indices_.begin(), index); it != std::next(indices_.begin(), maxValidIndex_ + 1);
             it++) {
            if (*it == std::numeric_limits<uint32_t>::max())
                continue;

            if (pos == std::numeric_limits<uint32_t>::max()) {
                pos = *it;
            }

            (*it)++;
        }

        assert(pos < store_.size());

        indices_[index] = pos;

        it = store_.emplace(std::next(store_.cbegin(), pos), std::forward<Args>(args)...);
    }

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

    for (auto it = std::next(indices_.begin(), index); it != std::next(indices_.begin(), maxValidIndex_ + 1); it++) {
        if (*it == std::numeric_limits<uint32_t>::max())
            continue;

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
void TransformStorage<CHUNK_SIZE, INITIAL_CHUNK_COUNT>::_reserveStoreIfNecessary(uint32_t pos)
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
