//
// Created by anton on 6/19/25.
//

#ifndef CYCLONITE_CORE_HASH_H_
#define CYCLONITE_CORE_HASH_H_

#include <functional>
#include <type_traits>
#include <utility>

namespace cyclonite::core {
template<typename Key>
auto hash(Key&& key) -> size_t
{
    return std::hash<std::decay_t<Key>>{}(std::forward<Key>(key));
}

template<typename Key0, typename... KeyN>
auto hash(Key0 key0, KeyN&&... keyN) -> size_t
{
    auto h = core::hash(std::forward<Key0>(key0));
    ((h ^= core::hash(std::forward<KeyN>(keyN)) + 0x9e3779b9 + (h << 6) + (h >> 2)), ...);
    return h;
}
}

#endif //  CYCLONITE_CORE_HASH_H_
