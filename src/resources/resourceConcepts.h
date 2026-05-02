//
// Created by anton on 4/13/26.
//

#ifndef CYCLONITE_RESOURCES_RESOURCE_CONCEPTS_H
#define CYCLONITE_RESOURCES_RESOURCE_CONCEPTS_H

#include "managedResource.h"
#include "resourceGroupBase.h"
#include <concepts>
#include <future>
#include <metrix/type_traits.h>
#include <type_traits>

namespace cyclonite::resources {
template<typename T>
concept ManagedResourceConcept = requires(T t) { requires std::is_base_of_v<ManagedResource<T>, T>; };

template<typename T>
concept CustomSourceConcept = requires(T t) {
    { t.load() } -> std::same_as<std::future<void>>;

    requires std::is_move_constructible_v<T>;
};
}

#endif // CYCLONITE_RESOURCES_RESOURCE_CONCEPTS_H
