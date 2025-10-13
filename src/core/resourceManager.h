//
// Created by anton on 6/13/25.
//

#ifndef GFX_RESOURCE_MANAGER_H
#define GFX_RESOURCE_MANAGER_H

#include "resourceUniqueRef.h"
#include <array>
#include <cassert>
#include <deque>
#include <metrix/type_list.h>
#include <numeric>
#include <type_traits>
#include <vector>

namespace cyclonite::core {
class ResourceManagerBase
{
    friend class ResourceBase;

public:
    ResourceManagerBase() = default;

    [[nodiscard]] virtual auto isResourceValid(ResourceId id) const -> bool = 0;

    virtual ~ResourceManagerBase() = default;

    [[deprecated]]
    void setCurrentFrame(uint_fast64_t frameNumber)
    {
        currentFrame_ = frameNumber;
    }

    void setLastCompletedFrame(uint_fast64_t frameNumber) { lastCompletedFrame_ = frameNumber; }

    [[nodiscard]] auto currentFrame() const -> uint_fast64_t { return currentFrame_; }
    [[nodiscard]] auto lastCompletedCurrentFrame() const -> uint_fast64_t { return lastCompletedFrame_; }

protected:
    virtual void releaseResourceImmediate(ResourceId id) = 0;

    [[deprecated]]
    virtual void releaseResourceDeferred(ResourceId id) = 0;

    static auto makeUniqueRef(ResourceBase* resource) -> ResourceUniqueRef
    {
        return ResourceUniqueRef{ resource->resourceId(), resource };
    }

private:
    uint_fast64_t currentFrame_;
    uint_fast64_t lastCompletedFrame_;
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
}

template<typename T>
concept ResourceConcept = std::is_base_of_v<ResourceBase, T>;

template<ResourceConcept... ResourceTypes>
class ResourceManager final : public ResourceManagerBase
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
          , index{ std::numeric_limits<uint32_t>::max() }
          , type{ std::numeric_limits<uint16_t>::max() }
        {
        }

        resource_deleter_f deleter;
        uint32_t version;
        uint32_t index;
        uint16_t type;
    };

    struct resource_storage_t
    {
        std::array<std::vector<resource_block_t>, resource_meta_t::resource_type_count_v> resources = {};
        std::array<std::vector<uint32_t>, resource_meta_t::resource_type_count_v> freeIndices = {};
    };

public:
    friend class ResourceBase;

    ResourceManager() = default;

    ResourceManager(ResourceManager const&) = delete;

    ResourceManager(ResourceManager&&) = default;

    ~ResourceManager() override;

    auto operator=(ResourceManager const&) -> ResourceManager& = delete;

    auto operator=(ResourceManager&&) -> ResourceManager& = default;

    template<typename ResourceType, typename... Args>
    auto allocResource(Args&&... args) -> ResourceUniqueRef
        requires(resource_type_list_t::template has_type<ResourceType>::value);

    [[deprecated]]
    void gc(bool clearAll = false);

    [[nodiscard]] auto isResourceValid(ResourceId id) const -> bool override;

protected:
    void releaseResourceImmediate(ResourceId id) override;

    [[deprecated]]
    void releaseResourceDeferred(ResourceId id) override;

    auto alloc(uint16_t type) -> std::pair<uint32_t, uint32_t>;

    void free(uint32_t index);

    std::vector<resource_block_header_t> headers_;
    std::vector<uint32_t> emptyHeaders_;
    std::deque<std::pair<uint64_t, uint32_t>> garbage_;
    resource_storage_t storage_;
};

template<ResourceConcept... ResourceTypes>
ResourceManager<ResourceTypes...>::~ResourceManager()
{
    gc(true);
}

template<ResourceConcept... ResourceTypes>
auto ResourceManager<ResourceTypes...>::alloc(uint16_t type) -> std::pair<uint32_t, uint32_t>
{
    assert(type < resource_meta_t::resource_type_count_v);

    auto blockIndex = std::numeric_limits<uint32_t>::max();

    if (storage_.freeIndices[type].empty()) {
        blockIndex = storage_.freeIndices[type].size();
        storage_.resources[type].emplace_back();
    } else {
        blockIndex = storage_.freeIndices[type].back();
        storage_.resources[type].pop_back();
    }
    assert(blockIndex < storage_.resources[type].size());

    auto headerIndex = std::numeric_limits<uint32_t>::max();
    if (!emptyHeaders_.empty()) {
        headerIndex = emptyHeaders_.size();
        emptyHeaders_.emplace_back();
    } else {
        headerIndex = emptyHeaders_.back();
        emptyHeaders_.pop_back();
    }
    assert(headerIndex < headers_.size());

    return std::pair{ headerIndex, blockIndex };
}

template<ResourceConcept... ResourceTypes>
void ResourceManager<ResourceTypes...>::free(uint32_t index)
{
    auto& header = headers_[index];
    auto idx = header.index;
    auto type = header.type;

    header.deleter(storage_.resources[type][idx].bytes);

    storage_.freeIndices[type].push_back(idx);

    header.version++;
    header.type = std::numeric_limits<uint16_t>::max();
    header.index = std::numeric_limits<uint32_t>::max();
    header.deleter = [](void*) -> void {};

    emptyHeaders_.push_back(index);
}

template<ResourceConcept... ResourceTypes>
template<typename ResourceType, typename... Args>
auto ResourceManager<ResourceTypes...>::allocResource(Args&&... args) -> ResourceUniqueRef
    requires(resource_type_list_t::template has_type<ResourceType>::value)
{
    auto type = resource_meta_t::template type_index_v<ResourceType>();
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

    return makeUniqueRef(r->resourceBase());
}

template<ResourceConcept... ResourceTypes>
auto ResourceManager<ResourceTypes...>::isResourceValid(ResourceId id) const -> bool
{
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

// TODO:: remove deferred release
template<ResourceConcept... ResourceTypes>
void ResourceManager<ResourceTypes...>::releaseResourceDeferred(ResourceId id)
{
    assert(isResourceValid(id));
    garbage_.emplace_back(static_cast<uint64_t>(currentFrame()), id.index());
}

template<ResourceConcept... ResourceTypes>
void ResourceManager<ResourceTypes...>::gc(bool clearAll)
{
    auto condition =
      clearAll ? std::numeric_limits<uint64_t>::max() : static_cast<uint64_t>(lastCompletedCurrentFrame());

    do {
        if (garbage_.empty())
            break;
        auto [frame, index] = garbage_.front();

        if (frame <= condition) {
            garbage_.pop_front();
            free(index);
        } else {
            break;
        }
    } while (true);
}
}

#endif // GFX_RESOURCE_MANAGER_H
