//
// Created by anton on 6/13/25.
//

#ifndef GFX_RESOURCE_MANAGER_H
#define GFX_RESOURCE_MANAGER_H

#include "core/configTraitMacro.h"
#include "resourceSharedRef.h"
#include "resourceUniqueRef.h"
#include <array>
#include <bitset>
#include <cassert>
#include <chrono>
#include <deque>
#include <metrix/type_list.h>
#include <shared_mutex>
#include <mutex>
#include <numeric>
#include <type_traits>
#include <unordered_map>
#include <vector>

namespace cyclonite::core {
class ResourceManagerBase
{
    friend class ResourceBase;

public:
    ResourceManagerBase() = default;

    [[nodiscard]] virtual auto isResourceValid(ResourceId id) const -> bool = 0;

    virtual ~ResourceManagerBase() = default;

protected:
    virtual void releaseResourceImmediate(ResourceId id) = 0;

    virtual void releaseResourceDeferred(ResourceId id) = 0;

    static auto makeUniqueRef(ResourceBase* resource) -> ResourceUniqueRef
    {
        return ResourceUniqueRef{ resource->resourceId(), resource };
    }
};

namespace internal {
template<size_t TypeAlign, size_t... TypeAligns>
inline constexpr auto get_uniform_align() -> size_t
{
    if constexpr (sizeof...(TypeAligns) > 0) {
        auto result = TypeAlign;
        return ((result = std::lcm(result, TypeAligns)), ...);
    } else {
        return TypeAlign;
    }
}

template<size_t TypeSize, size_t... TypeSizes>
inline constexpr auto get_uniform_size() -> size_t
{
    if constexpr (sizeof...(TypeSizes) > 0) {
        auto result = TypeSize;
        return ((result = std::max(result, TypeSizes)), ...);
    } else {
        return TypeSize;
    }
}

template<typename T>
struct resource_traits_decl
{
    using yes_t = uint8_t;
    using no_t = uint16_t;

    DECLARE_CONFIG_TRAIT(items_per_chunk_count, uint16_t, 256)
};

template<typename T>
struct resource_traits
{
    static constexpr auto items_per_chunk_count_v = resource_traits_decl<T>::items_per_chunk_count();
};
} // internal

template<typename T>
concept ResourceConcept = std::is_base_of_v<ResourceBase, T>;

template<ResourceConcept... ResourceTypes>
class ResourceManager : public ResourceManagerBase
{
    template<typename ResourceList>
    struct ResourceMeta;

    template<ResourceConcept... Resources>
    struct ResourceMeta<metrix::type_list<Resources...>>
    {
        static constexpr size_t resource_type_count_v = sizeof...(Resources);

        static constexpr size_t uniform_align_v = internal::get_uniform_align<alignof(Resources)...>();

        static constexpr size_t uniform_size_v = internal::get_uniform_size<sizeof(Resources)...>();

        template<typename Res>
        static constexpr auto type_index_v() -> size_t
        {
            return metrix::type_list<Resources...>::template get_type_index<Res>::value;
        }

        static constexpr size_t invalid_resource_v = std::numeric_limits<size_t>::max();
    };

    using resource_type_list_t = metrix::type_list<ResourceTypes...>;

    using resource_meta_t = ResourceMeta<resource_type_list_t>;

    struct resource_block_t
    {
        alignas(resource_meta_t::uniform_align_v) std::byte bytes[resource_meta_t::uniform_size_v] = {};
    };

    using resource_deleter_f = void (*)(void*);

    struct resource_block_header_t
    {
        resource_block_header_t()
          : deleter{ [](void*) -> void {} } // empty deleter
          , version{ 1 }
          , chunk{ std::numeric_limits<uint16_t>::max() }
          , index{ std::numeric_limits<uint16_t>::max() }
          , type{ std::numeric_limits<uint8_t>::max() }
        {
        }

        resource_deleter_f deleter;
        uint32_t version;
        uint16_t chunk;
        uint16_t index;
        uint8_t type;
    };

    struct resource_storage_chunk_t
    {
        explicit resource_storage_chunk_t(uint16_t size = 255)
          : resources{}
          , freeIndices{}
        {
            resources.resize(size, resource_block_t{});
            freeIndices.resize(size, 0);

            std::iota(freeIndices.begin(), freeIndices.end(), 0);
        }

        std::vector<resource_block_t> resources;
        std::deque<uint32_t> freeIndices;
    };

    struct resource_storage_t
    {
        std::array<std::unordered_map<uint16_t, resource_storage_chunk_t>, resource_meta_t::resource_type_count_v>
          chunks = {};
    };

    struct resource_allocation_info_t
    {
        resource_allocation_info_t()
          : headerIndex{ std::numeric_limits<uint32_t>::max() }
          , chunkIndex{ std::numeric_limits<uint16_t>::max() }
          , blockIndex{ std::numeric_limits<uint16_t>::max() }
          , typeIndex{ std::numeric_limits<uint8_t>::max() }
        {
        }

        resource_allocation_info_t(uint32_t header, uint8_t type, uint16_t chunk, uint16_t index)
          : headerIndex{ header }
          , chunkIndex{ chunk }
          , blockIndex{ index }
          , typeIndex{ type }
        {
        }

        uint32_t headerIndex;
        uint16_t chunkIndex;
        uint16_t blockIndex;
        uint8_t typeIndex;
    };

public:
    friend class ResourceBase;

    ResourceManager() = default;

    ResourceManager(ResourceManager const&) = delete;

    ResourceManager(ResourceManager&&) = delete;

    ~ResourceManager() override;

    auto operator=(ResourceManager const&) -> ResourceManager& = delete;

    auto operator=(ResourceManager&&) -> ResourceManager& = delete;

    template<typename ResourceType, typename... Args>
    auto allocResource(Args&&... args) -> ResourceUniqueRef
        requires(resource_type_list_t::template has_type<ResourceType>::value);

    void gc(bool clearAll = false);

    [[nodiscard]] auto isResourceValid(ResourceId id) const -> bool final;

protected:
    void releaseResourceImmediate(ResourceId id) override;

    void releaseResourceDeferred(ResourceId id) override;

    template<typename ResourceType>
    auto alloc() -> resource_allocation_info_t;

    void free(uint32_t headerIndex);

    mutable std::shared_mutex headersGuard_;

    std::vector<resource_block_header_t> headers_;
    std::vector<uint32_t> emptyHeaders_;
    resource_storage_t storage_;
    std::deque<std::pair<std::chrono::time_point<std::chrono::high_resolution_clock>, uint32_t>> garbage_;

public:
    template<bool isConst, typename... Res>
    class ResourceList
    {
        friend class ResourceManager;

    public:
        class Iterator
        {
            friend class ResourceList;

        public:
            using iterator_category = std::input_iterator_tag;
            using value_type = std::pair<ResourceSharedRef, uint8_t>;
            using difference_type = size_t;
            using pointer = value_type*;
            using reference = value_type&;

            auto operator++() -> Iterator&;

            auto operator*() const -> std::pair<ResourceSharedRef, uint8_t>;

            auto operator==(Iterator const& rhs) const -> bool { return list_ == rhs.list_ && cursor_ == rhs.cursor_; }
            auto operator!=(Iterator const& rhs) const -> bool { return list_ != rhs.list_ || cursor_ != rhs.cursor_; }

        private:
            using resource_list_t = std::conditional_t<isConst, ResourceList const*, ResourceList*>;

            Iterator(resource_list_t resList, size_t cursor)
              : cursor_{ cursor }
              , list_{ resList }
            {
            }

            void next();

            size_t cursor_;
            resource_list_t list_;
        };

        auto begin() const -> Iterator
        {
            auto it = Iterator{ this, 0 };
            it.next();

            return it;
        }

        auto begin() -> Iterator
        {
            auto it = Iterator{ this, 0 };
            it.next();

            return it;
        }

        auto end() const -> Iterator
        {
            auto lock = std::shared_lock{ manager_->headersGuard_ };
            auto size = manager_->headers_.size();
            return Iterator{ this, size };
        }

        auto end() -> Iterator
        {
            auto lock = std::shared_lock{ manager_->headersGuard_ };
            auto size = manager_->headers_.size();
            return Iterator{ this, size };
        }

    private:
        using manager_ref_t = std::conditional_t<isConst, ResourceManager const&, ResourceManager&>;
        using manager_ptr_t = std::conditional_t<isConst, ResourceManager const*, ResourceManager*>;

        explicit ResourceList(manager_ref_t& manager)
          : manager_{ &manager }
        {
        }

        manager_ptr_t manager_;
    };

    template<typename... Res>
    [[nodiscard]] auto resourceList() -> ResourceList<false, Res...>
    {
        return ResourceList<false, Res...>{ *this };
    }

    template<typename... Res>
    [[nodiscard]] auto resourceList() const -> ResourceList<true, Res...>
    {
        return ResourceList<true, Res...>{ *this };
    }
};

template<ResourceConcept... ResourceTypes>
ResourceManager<ResourceTypes...>::~ResourceManager()
{
    gc(true);
}

template<ResourceConcept... ResourceTypes>
template<typename ResourceType>
auto ResourceManager<ResourceTypes...>::alloc() -> resource_allocation_info_t
{
    auto type = resource_meta_t::template type_index_v<ResourceType>();
    auto headerIndex = std::numeric_limits<uint32_t>::max();

    if (emptyHeaders_.empty()) {
        headerIndex = headers_.size();
        headers_.emplace_back();
    } else {
        headerIndex = emptyHeaders_.back();
        emptyHeaders_.pop_back();
    }
    assert(headerIndex < headers_.size());

    auto resourceChunkIndex = std::numeric_limits<uint16_t>::max();
    auto resourceBlockIndex = std::numeric_limits<uint16_t>::max();

    auto& chunks = storage_.chunks[type];
    for (auto& [id, chunk] : chunks) {
        if (!chunk.freeIndices.empty()) {
            resourceChunkIndex = id;
            break;
        }
    }

    if (resourceChunkIndex == std::numeric_limits<uint16_t>::max()) {
        auto chunkIndexNew = chunks.size();
        [[maybe_unused]] auto [_, success] = chunks.emplace(
          chunkIndexNew, resource_storage_chunk_t{ internal::resource_traits<ResourceType>::items_per_chunk_count_v });
        assert(success);

        resourceChunkIndex = chunkIndexNew;
    }

    assert(resourceChunkIndex != std::numeric_limits<uint16_t>::max());
    auto& chunk = chunks[resourceChunkIndex];
    resourceBlockIndex = chunk.freeIndices.back();
    chunk.freeIndices.pop_back();
    assert(resourceBlockIndex < chunk.resources.size());

    return resource_allocation_info_t{
        headerIndex, static_cast<uint8_t>(type), resourceChunkIndex, resourceBlockIndex
    };
}

template<ResourceConcept... ResourceTypes>
template<typename ResourceType, typename... Args>
auto ResourceManager<ResourceTypes...>::allocResource(Args&&... args) -> ResourceUniqueRef
    requires(resource_type_list_t::template has_type<ResourceType>::value)
{
    auto version = std::numeric_limits<uint32_t>::max();
    auto allocationInfo = resource_allocation_info_t{};

    {
        auto lock = std::unique_lock{ headersGuard_ }; // guard header ref
        allocationInfo = alloc<ResourceType>();

        auto& header = headers_[allocationInfo.headerIndex];
        header.chunk = allocationInfo.chunkIndex;
        header.index = allocationInfo.blockIndex;
        header.type = allocationInfo.typeIndex;
        header.deleter = [](void* ptr) -> void {
            auto* p = std::launder(reinterpret_cast<ResourceType*>(ptr));
            std::destroy_at(p);
        };

        version = header.version;
    }

    auto resourceId = ResourceId{ allocationInfo.headerIndex, version };

    auto& chunks = storage_.chunks[allocationInfo.typeIndex];
    auto& chunk = chunks[allocationInfo.chunkIndex];
    auto* memory = chunk.resources[allocationInfo.blockIndex].bytes;
    auto* r = new (memory) ResourceType(this, resourceId, std::forward<Args>(args)...);

    return makeUniqueRef(r->resourceBase());
}

template<ResourceConcept... ResourceTypes>
void ResourceManager<ResourceTypes...>::free(uint32_t headerIndex)
{
    auto lock = std::unique_lock{ headersGuard_ };

    auto& header = headers_[headerIndex];
    auto chunk = header.chunk;
    auto idx = header.index;
    auto type = header.type;

    auto& chunks = storage_.chunks[type];

    header.deleter(chunks[chunk].resources[idx].bytes);

    chunks[chunk].freeIndices.push_back(idx);

    header.version++;
    header.type = std::numeric_limits<uint8_t>::max();
    header.chunk = std::numeric_limits<uint16_t>::max();
    header.index = std::numeric_limits<uint16_t>::max();
    header.deleter = [](void*) -> void {};

    emptyHeaders_.push_back(headerIndex);
}

template<ResourceConcept... ResourceTypes>
auto ResourceManager<ResourceTypes...>::isResourceValid(ResourceId id) const -> bool
{
    auto lock = std::shared_lock{ headersGuard_ };

    assert(id.index() < headers_.size());
    auto const& header = headers_[id.index()];

    return id.version() == header.version;
}

template<ResourceConcept... ResourceTypes>
void ResourceManager<ResourceTypes...>::releaseResourceImmediate(ResourceId id)
{
    assert(isResourceValid(id));
    free(id.index());
}

template<ResourceConcept... ResourceTypes>
void ResourceManager<ResourceTypes...>::releaseResourceDeferred(ResourceId id)
{
    auto lock = std::unique_lock{ headersGuard_ };

    assert(isResourceValid(id));
    garbage_.emplace_back(std::chrono::high_resolution_clock::now(), id.index());
}

template<ResourceConcept... ResourceTypes>
void ResourceManager<ResourceTypes...>::gc(bool clearAll)
{
    auto lock = std::unique_lock{ headersGuard_ };

    do {
        if (garbage_.empty())
            break;
        auto [tp, index] = garbage_.front();

        auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - tp);
        // TODO:: temporal gc logic
        if (ms.count() >= 64 || clearAll) {
            garbage_.pop_front();
            free(index);
        } else {
            break;
        }
    } while (true);
}

// ResourceList::Iterator::

template<ResourceConcept... ResourceTypes>
template<bool isConst, typename... Res>
void ResourceManager<ResourceTypes...>::ResourceList<isConst, Res...>::Iterator::next()
{
    auto lock = std::shared_lock{ list_->manager_->headersGuard_ };

    auto const& headers = list_->manager_->headers_;
    auto const size = headers.size();

    while (cursor_ < size) {
        auto const& header = headers[cursor_];
        auto [_0, _1, chunk, index, type] = header;

        if (type != std::numeric_limits<uint8_t>::max() && chunk != std::numeric_limits<uint16_t>::max() &&
            index != std::numeric_limits<uint16_t>::max()) { // any valid element
            if constexpr (sizeof...(Res) > 0) {
                if (((resource_meta_t::template type_index_v<Res>() == static_cast<size_t>(type)) ||
                     ...)) { // valid element (filtered)
                    break;
                }
            } else { // any valid element (no filters)
                break;
            }
        }

        cursor_++;
    } // all headers
}

template<ResourceConcept... ResourceTypes>
template<bool isConst, typename... Res>
auto ResourceManager<ResourceTypes...>::ResourceList<isConst, Res...>::Iterator::operator*() const
  -> std::pair<ResourceSharedRef, uint8_t>
{
    auto lock = std::shared_lock{ list_->manager_->headersGuard_ };

    auto const& headers = list_->manager_->headers_;
    auto const& storage = list_->manager_->storage_;

    auto [_0, _1, chunk, index, type] = headers[cursor_];

    auto const& chunks = storage.chunks[type];

    assert(chunks.contains(chunk));

    auto res = const_cast<ResourceBase*>(
      reinterpret_cast<ResourceBase const*>(std::launder(chunks.at(chunk).resources[index].bytes)));

    return std::pair{ makeResourceSharedRefUnsafe(res), type };
}

template<ResourceConcept... ResourceTypes>
template<bool isConst, typename... Res>
auto ResourceManager<ResourceTypes...>::ResourceList<isConst, Res...>::Iterator::operator++() -> Iterator&
{
    cursor_++;
    next();
    return *this;
}
}

#endif // GFX_RESOURCE_MANAGER_H
