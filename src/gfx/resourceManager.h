//
// Created by anton on 6/13/25.
//

#ifndef GFX_RESOURCE_MANAGER_H
#define GFX_RESOURCE_MANAGER_H

#include "resourceTypeList.h"
#include <array>
#include <numeric>
#include <vector>

namespace cyclonite::gfx {
namespace internal {
template<size_t TypeAlign, size_t... TypeAligns>
inline constexpr auto get_uniform_align() -> size_t
{
    auto result = TypeAlign;
    return ((result = std::lcm(result, TypeAligns)), ...);
}

template<size_t TypeSize, size_t... TypeSizes>
inline constexpr auto get_uniform_size() -> size_t
{
    auto result = TypeSize;
    return ((result = std::max(result, TypeSizes)), ...);
}
}

class ResourceManager
{
    template<typename ResourceList>
    struct ResourceMeta;

    template<typename... Resources>
    struct ResourceMeta<metrix::type_list<Resources...>>
    {
        static constexpr size_t resource_type_count_v = sizeof...(Resources);

        static constexpr size_t uniform_align_v =
          internal::get_uniform_align<alignof(type_traits::platform_implementation_t<Resources>)...>();

        static constexpr size_t uniform_size_v =
          internal::get_uniform_size<sizeof(type_traits::platform_implementation_t<Resources>)...>();

        template<typename Res>
        static constexpr auto type_index_v() -> size_t
        {
            return metrix::type_list<Resources...>::template get_type_index<Res>::value;
        }

        static constexpr size_t invalid_resource_v = std::numeric_limits<size_t>::max();
    };

    using resource_meta_t = ResourceMeta<resource_type_list_t>;

    using resource_block_t = struct
    {
        alignas(resource_meta_t::uniform_align_v) std::byte bytes[resource_meta_t::uniform_size_v] = {};
    };

    using resource_deleter_f = void (*)(void*);

    struct resource_block_header_t
    {
        resource_block_header_t()
          : deleter{ [](void*) -> void {} } // empty deleter
          , version{ 1 }
          , index{ std::numeric_limits<uint32_t>::max() }
          , type{ std::numeric_limits<uint16_t>::max() }
        {
        }

        resource_deleter_f deleter;
        uint32_t version;
        uint32_t index;
        uint16_t type;
    };

    using resource_storage_t = struct
    {
        std::array<std::vector<resource_block_t>, resource_meta_t::resource_type_count_v> resources = {};
        std::array<std::vector<uint32_t>, resource_meta_t::resource_type_count_v> freeIndices = {};
    };

private:
    std::vector<resource_block_header_t> headers_;
    std::vector<uint32_t> emptyHeaders_;
    resource_storage_t storage_;
};
}

#endif // GFX_RESOURCE_MANAGER_H
