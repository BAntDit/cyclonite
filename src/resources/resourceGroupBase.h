//
// Created by anton on 3/24/26.
//

#ifndef CYCLONITE_RESOURCES_RESOURCE_GROUP_BASE_H
#define CYCLONITE_RESOURCES_RESOURCE_GROUP_BASE_H

#include "managedResource.h"
#include <concepts>
#include <future>
#include <metrix/type_traits.h>
#include <type_traits>

namespace cyclonite::resources {
class ResourceGroupBase
{
public:
    virtual ~ResourceGroupBase() = default;
};

template<typename T>
concept ManagedResourceConcept = requires(T t) {
    requires std::is_move_constructible_v<T> || std::is_copy_constructible_v<T>;

    requires std::is_base_of_v<ManagedResource<T>, T>;
};

template<typename T>
concept CustomSourceConcept = requires(T t) {
    requires std::is_member_function_pointer_v<decltype(&T::setResourceGroup)> &&
               std::is_base_of_v<ResourceGroupBase,
                                 typename metrix::member_function_argument_type_list_t<
                                   decltype(&T::setResourceGroup)>::template get_type<0>::type>;

    { t.load() } -> std::same_as<std::future<void>>;

    requires std::is_move_constructible_v<T>;
};
}

#endif // CYCLONITE_RESOURCES_RESOURCE_GROUP_BASE_H