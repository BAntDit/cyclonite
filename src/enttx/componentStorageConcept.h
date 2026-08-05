//
// Created by anton on 8/5/26.
//

#ifndef CYCLONITE_ENTTX_COMPONENT_STORAGE_CONCEPT_H
#define CYCLONITE_ENTTX_COMPONENT_STORAGE_CONCEPT_H

#include <concepts>
#include <cstddef>
#include <cstdint>
#include <utility>

namespace cyclonite::enttx {
namespace internal
{
template<typename T>
concept has_iterator_traits = requires(T)
{
    typename T::iterator;

    typename T::const_iterator;
};
}

template<typename T>
concept ComponentConcept = std::is_default_constructible_v<T>;

template<typename T>
concept ComponentStorageConcept = requires(T& t, T const& ct, uint32_t index) {
    typename T::component_type;

    typename T::storage_type;

    requires internal::has_iterator_traits<typename T::storage_type>;

    requires std::is_default_constructible_v<T>;

    { t.destroy(index) } -> std::same_as<void>;

    { ct.capacity() } -> std::same_as<size_t>;

    { ct.size() } -> std::same_as<size_t>;

    { t.get(index) } -> std::same_as<typename T::component_type&>;

    { ct.get(index) } -> std::same_as<typename T::component_type const&>;

    { t.create(index) } -> std::same_as<typename T::component_type&>;

    { t.begin() } -> std::same_as<typename T::storage_type::iterator>;

    { ct.begin() } -> std::same_as<typename T::storage_type::const_iterator>;

    { t.end() } -> std::same_as<typename T::storage_type::iterator>;

    { ct.end() } -> std::same_as<typename T::storage_type::const_iterator>;
};
}

#endif // CYCLONITE_ENTTX_COMPONENT_STORAGE_CONCEPT_H