//
// Created by bantdit on 2/8/20.
//

#ifndef CYCLONITE_HASH_H
#define CYCLONITE_HASH_H

#include <cstddef>
#include <functional>
#include <tuple>

namespace cyclonite {
template<typename Tuple, size_t... idx>
auto tuple_hash(Tuple&& tuple, std::index_sequence<idx...> &&) -> size_t;

template<typename T>
auto hash_value(T&& value) -> size_t
{
    return std::hash<std::decay_t<T>>{}(std::forward<T>(value));
}

template<typename... Args>
auto hash_value(std::tuple<Args...> const& value) -> size_t
{
    return tuple_hash(value, std::make_index_sequence<sizeof...(Args)>{});
}

template<typename Tuple, size_t... idx>
auto tuple_hash(Tuple&& tuple, std::index_sequence<idx...> &&) -> size_t
{
    size_t result = 0;

    for (auto&& hash : std::array<size_t, sizeof...(idx)>{ hash_value(std::get<idx>(std::forward<Tuple>(tuple)))... }) {
        result ^= hash + 0x9e3779b9 + (result << 6) + (result >> 2);
    }

    return result;
}

struct hash
{
    template<typename T>
    auto operator()(T&& value) const -> size_t
    {
        return hash_value(std::forward<T>(value));
    }
};
}

#endif // CYCLONITE_HASH_H
