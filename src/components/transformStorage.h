//
// Created by bantdit on 1/13/20.
//

#ifndef CYCLONITE_TRANSFORMSTORAGE_H
#define CYCLONITE_TRANSFORMSTORAGE_H

#include <enttx/enttx.h>
#include "transform.h"

namespace cyclonite::components {
template<size_t CHUNK_SIZE, size_t INITIAL_CHUNK_COUNT>
class TransformStorage: public enttx::BaseComponentStorage<TransformStorage<CHUNK_SIZE, INITIAL_CHUNK_COUNT>, Transform>
{
public:
    TransformStorage();

    auto get(uint32_t index) const -> Transform const&;

    auto get(uint32_t index) -> Transform&;

    template<typename... Args>
    auto create(uint32_t index, Args&&... args) -> Transform&;
private:
    std::vector<uint32_t> indices_;
    std::vector<Transform> store_;
};

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
    return const_cast<Transform&>(std::as_const(*this).get(index));
}

template<size_t CHUNK_SIZE, size_t INITIAL_CHUNK_COUNT>
template<typename... Args>
auto TransformStorage<CHUNK_SIZE, INITIAL_CHUNK_COUNT>::create(uint32_t index, Args&&... args) -> Transform&
{
    // TODO:: ...
}
}

#endif // CYCLONITE_TRANSFORMSTORAGE_H
