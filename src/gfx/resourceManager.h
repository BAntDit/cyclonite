//
// Created by anton on 6/13/25.
//

#ifndef GFX_RESOURCE_MANAGER_H
#define GFX_RESOURCE_MANAGER_H

#include "resourceRef.h"
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

public:
    friend class ResourceBase;

    ResourceManager() = default;

    ResourceManager(ResourceManager const&) = delete;

    ResourceManager(ResourceManager&&) = default;

    ~ResourceManager();

    auto operator=(ResourceManager const&) -> ResourceManager& = delete;

    auto operator=(ResourceManager&&) -> ResourceManager& = default;

    template<typename ResourceType, typename... Args>
    auto allocResource(Args&&... args) -> ResourceRef
        requires(resource_type_list_t::has_type<ResourceType>::value);

    void gc(bool clearAll = false);

    [[nodiscard]] auto isResourceValid(ResourceId id) const -> bool;

private:
    void releaseResourceImmediate(ResourceId id);

    void releaseResourceDeferred(ResourceId id);

    auto alloc(uint16_t type) -> std::pair<uint32_t, uint32_t>;

    std::vector<resource_block_header_t> headers_;
    std::vector<uint32_t> emptyHeaders_;
    resource_storage_t storage_;
};

template<typename ResourceType, typename... Args>
auto ResourceManager::allocResource(Args&&... args) -> ResourceRef
    requires(resource_type_list_t::has_type<ResourceType>::value)
{
    auto type = resource_meta_t::type_index_v<ResourceType>();
    auto [headerIndex, blockIndex] = alloc(type);

    auto& header = headers_[headerIndex];
    header.type = type;
    header.index = blockIndex;
    header.deleter = [](void* ptr) -> void {
        auto* p = std::launder(reinterpret_cast<ResourceType*>(ptr));
        std::destroy_at(p);
    };

    auto resourceId = ResourceId{ headerIndex, header.version };

    auto* memory = storage_.resources[type][blockIndex].bytes;
    auto* r = new (memory) ResourceType(this, resourceId, std::forward<Args>(args)...);

    return ResourceRef{ resourceId, r };
}
}

#endif // GFX_RESOURCE_MANAGER_H
