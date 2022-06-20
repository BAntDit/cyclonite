//
// Created by anton on 5/23/22.
//

#ifndef CYCLONITE_RESOURCEMANAGER_H
#define CYCLONITE_RESOURCEMANAGER_H

#include "resource.h"
#include <limits>
#include <set>
#include <tuple>
#include <vector>

namespace cyclonite::resources {
template<typename R, size_t N, size_t M>
struct resource_reg_info_t
{};

namespace internal {
template<typename T, template<typename, size_t, size_t> typename ResourceRegInfo>
struct is_resource_reg_info_specialization : std::false_type
{};

template<template<typename, size_t, size_t> typename ResourceRegInfo, typename T, size_t N, size_t M>
struct is_resource_reg_info_specialization<ResourceRegInfo<T, N, M>, ResourceRegInfo> : std::true_type
{};

template<typename T, template<typename, size_t, size_t> typename ResourceRegInfo>
inline constexpr bool is_resource_reg_info_specialization_v =
  is_resource_reg_info_specialization<T, ResourceRegInfo>::value;
}

template<typename T>
concept ResourceRegInfoSpecialization = internal::is_resource_reg_info_specialization_v<T, resource_reg_info_t>;

template<typename T>
concept ResourceTypeConcept = std::derived_from<T, Resource>;

class ResourceManager
{
    friend class Resource;

public:
    explicit ResourceManager(size_t expectedResourceCount);

    ResourceManager(ResourceManager const&) = delete;

    ResourceManager(ResourceManager&&) = default;

    ~ResourceManager();

    auto operator=(ResourceManager const&) -> ResourceManager& = delete;

    auto operator=(ResourceManager &&) -> ResourceManager& = default;

    template<ResourceTypeConcept R, uint32_t InitialCapacity, size_t InitialDynamicBufferSize>
    void registerDynamicSizeResource();

    template<ResourceTypeConcept R, uint32_t InitialCapacity>
    void registerFixedSizeResource();

    void registerResources(ResourceRegInfoSpecialization auto&&... regInfo);

    template<ResourceTypeConcept R, typename... Args>
    auto create(Args&&... args) -> Resource::Id;

    void erase(Resource::Id id);

    [[nodiscard]] auto isValid(Resource::Id id) const -> bool;

    [[nodiscard]] auto isValid(Resource const& resource) const -> bool;

    [[nodiscard]] auto get(Resource::Id id) const -> Resource const&;

    auto get(Resource::Id id) -> Resource&;

    template<ResourceTypeConcept R>
    [[nodiscard]] auto getAs(Resource::Id id) const -> R const&
    {
        return get(id).as<R>();
    }

    template<ResourceTypeConcept R>
    auto getAs(Resource::Id id) -> R&
    {
        return get(id).as<R>();
    }

    [[nodiscard]] auto getResourceDynamicBufferSize(Resource::ResourceTag resourceTag) const -> size_t;

    [[nodiscard]] auto getResourceDynamicBufferFreeSize(Resource::ResourceTag resourceTag) const -> size_t;

private:
    template<typename R, size_t N, size_t M>
    void registerResource(resource_reg_info_t<R, N, M>);

    auto allocResource(Resource::ResourceTag tag, size_t size) -> Resource::Id;

    auto allocDynamicBuffer(Resource::ResourceTag tag, size_t size, bool resizeAllowed = true) -> size_t;

    void freeDynamicBuffer(uint16_t dynamicIndex, size_t offset, size_t size);

    auto getDynamicData(Resource::Id id) -> std::byte*;

    void resizeDynamicBuffer(Resource::ResourceTag tag, size_t additionalSize);

private:
    struct free_range_comparator
    {
        auto operator()(std::pair<size_t, size_t> const& a, std::pair<size_t, size_t> const& b) const -> bool
        {
            return a.second < b.second;
        }
    };

    using resource_storage_t = std::vector<std::byte>;
    using free_items_t = std::vector<uint32_t>;
    using free_ranges_t = std::set<std::pair<size_t, size_t>, free_range_comparator>;

    struct resource_t
    {
        using dynamic_buffer_realloc_func_t = void (*)(void*);

        resource_t() noexcept
          : size{ 0 }
          , version{ 0 }
          , item{ std::numeric_limits<uint32_t>::max() }
          , static_index{ std::numeric_limits<uint16_t>::max() }
          , dynamic_index{ std::numeric_limits<uint16_t>::max() }
        {}

        resource_t(size_t s,
                   uint32_t v,
                   uint32_t i,
                   uint16_t si,
                   uint16_t di) noexcept
          : size{ s }
          , version{ v }
          , item{ i }
          , static_index{ si }
          , dynamic_index{ di }
        {}

        size_t size;
        uint32_t version;
        uint32_t item;
        uint16_t static_index;
        uint16_t dynamic_index;
    };

    std::vector<resource_t> resources_;
    std::vector<uint32_t> freeResourceIndices_;
    std::vector<resource_storage_t> storages_;
    std::vector<free_items_t> freeItems_;
    std::vector<resource_storage_t> buffers_;
    std::vector<free_ranges_t> freeRanges_;
};

template<ResourceTypeConcept R, uint32_t InitialCapacity>
void ResourceManager::registerFixedSizeResource()
{
    R::type_tag().staticDataIndex = ++Resource::ResourceTag::_lastTagIndex;
    assert(R::type_tag().staticDataIndex == storages_.size());

    auto size = sizeof(R);

    storages_.template emplace_back(size * InitialCapacity, std::byte{ 0 });

    auto& freeItems = freeItems_.template emplace_back();
    freeItems.push_back(0); // first available item
}

template<ResourceTypeConcept R, uint32_t InitialCapacity, size_t InitialDynamicBufferSize>
void ResourceManager::registerDynamicSizeResource()
{
    registerFixedSizeResource<R, InitialCapacity>();

    R::type_tag().dynamicDataIndex = buffers_.size();
    buffers_.template emplace_back(InitialDynamicBufferSize, std::byte{ 0 });

    auto& freeRanges = freeRanges_.template emplace_back();
    freeRanges.insert(std::pair{ size_t{ 0 }, InitialDynamicBufferSize });
}

template<typename R, size_t N, size_t M>
void ResourceManager::registerResource(resource_reg_info_t<R, N, M>)
{
    if constexpr (M > 0) {
        registerDynamicSizeResource<R, N, M>();
    } else {
        registerFixedSizeResource<R, N>();
    }
}

void ResourceManager::registerResources(ResourceRegInfoSpecialization auto&&... regInfo)
{
    storages_.reserve(sizeof...(regInfo));
    freeItems_.reserve(sizeof...(regInfo));

    auto counter = []<typename R, size_t N, size_t M>(resource_reg_info_t<R, N, M>) -> uint16_t { return M > 0; };
    auto bufferCount = (... + counter(regInfo));

    buffers_.reserve(bufferCount);
    freeRanges_.reserve(bufferCount);

    (registerResource(std::forward<decltype(regInfo)>(regInfo)), ...);
}

template<ResourceTypeConcept R, typename... Args>
auto ResourceManager::create(Args&&... args) -> Resource::Id
{
    auto id = allocResource(R::type_tag_const(), sizeof(R));

    auto const [size, version, itemIndex, staticIndex, _] = resources_[id.index()];
    (void)version;
    (void)_;

    Resource* resource = new (storages_[staticIndex].data() + size * itemIndex) R(std::forward<Args>(args)...);

    resource->id_ = id;
    resource->resourceManager_ = this;

    if (resource->dynamicDataSize() > 0) {
        resource->dynamicOffset_ = allocDynamicBuffer(R::type_tag_const(), resource->dynamicDataSize());
    }

    return id;
}
}

#endif // CYCLONITE_RESOURCEMANAGER_H
