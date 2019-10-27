//
// Created by bantdit on 1/27/19.
//

#ifndef CYCLONITE_READARRAY_H
#define CYCLONITE_READARRAY_H

#include <cassert>
#include <nlohmann/json.hpp>

namespace cyclonite::loaders::gltf::internal {
template<typename T, size_t Size>
static auto readArray(nlohmann::json& array) -> std::array<T, Size>
{
    std::array<T, Size> result{};

    assert(array.is_array() && array.size() < Size);

    for (size_t i = 0; i < Size; i++) {
        result[i] = array.at(i).get<T>();
    }

    return result;
}
}

#endif // CYCLONITE_READARRAY_H
