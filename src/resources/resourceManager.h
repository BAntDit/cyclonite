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
template<typename R, size_t N, bool B>
struct resource_reg_info_t
{};

namespace internal {
template<typename T, template<typename, size_t, bool> typename ResourceRegInfo>
struct is_resource_reg_info_specialization : std::false_type
{};

template<template<typename, size_t, bool> typename ResourceRegInfo, typename T, size_t N, bool B>
struct is_resource_reg_info_specialization<ResourceRegInfo<T, N, B>, ResourceRegInfo> : std::true_type
{};

template<typename T, template<typename, size_t, bool> typename ResourceRegInfo>
inline constexpr bool is_resource_reg_info_specialization_v =
  is_resource_reg_info_specialization<T, ResourceRegInfo>::value;
}

template<typename T>
concept ResourceRegInfoSpecialization = internal::is_resource_reg_info_specialization_v<T, resource_reg_info_t>;

template<typename T>
concept ResourceTypeConcept = requires()
{
    {
        T {}
    }
    ->std::derived_from<Resource>;
    {
        T::type_tag()
    }
    ->std::same_as<Resource::ResourceTag&>;
    {
        T::type_tag_const()
    }
    ->std::same_as<Resource::ResourceTag const&>;
};

class ResourceManager
{
public:
    ResourceManager(size_t expectedResourceCount, size_t expectedDependencyCount);

    ResourceManager(ResourceManager const&) = delete;

    ResourceManager(ResourceManager&&) = default;

    ~ResourceManager() = default;

    auto operator=(ResourceManager const&) -> ResourceManager& = delete;

    auto operator=(ResourceManager &&) -> ResourceManager& = default;

    template<ResourceTypeConcept R, size_t InitialStorageSize>
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

private:
    template<typename R, size_t N, bool B>
    void registerResource(resource_reg_info_t<R, N, B>);

    auto allocResource(Resource::ResourceTag tag, size_t size) -> Resource::Id;

private:
    using resource_storage_t = std::vector<std::byte>;
    using free_ranges_t =
      std::set<std::pair<size_t, size_t>, decltype([](auto a, auto b) -> bool { return a.second < b.second; })>;

    struct resource_t
    {
        resource_t() noexcept
          : offset{ std::numeric_limits<size_t>::max() }
          , size{ 0 }
          , version{ 0 }
          , storage_index{ std::numeric_limits<uint16_t>::max() }
          , is_fixed_size{ false }
        {}

        resource_t(uint32_t v, uint16_t si, size_t ofs, size_t sz, bool is_fixed) noexcept
          : offset{ ofs }
          , size{ sz }
          , version{ v }
          , storage_index{ si }
          , is_fixed_size{ is_fixed }
        {}

        size_t offset;
        size_t size;
        uint32_t version;
        uint16_t storage_index;
        bool is_fixed_size;
    };

    std::vector<resource_t> resources_;
    std::vector<uint32_t> freeResourceIndices_;
    std::vector<ResourceDependency> dependencies_;
    std::vector<resource_storage_t> storages_;
    std::vector<free_ranges_t> freeRanges_;
};

template<ResourceTypeConcept R, size_t InitialStorageSize>
void ResourceManager::registerDynamicSizeResource()
{
    R::type_tag().isFixedSizePerItem = false;
    R::type_tag().index = ++Resource::ResourceTag::_lastTagIndex;
    assert(R::type_tag().index == storages_.size());

    storages_.template emplace_back(InitialStorageSize, std::byte{ 0 });
    freeRanges_.template emplace_back().insert(std::pair{ size_t{ 0 }, InitialStorageSize });
}

template<ResourceTypeConcept R, uint32_t InitialCapacity>
void ResourceManager::registerFixedSizeResource()
{
    R::type_tag().isFixedSizePerItem = true;
    R::type_tag().index = ++Resource::ResourceTag::_lastTagIndex;
    assert(R::type_tag().index == storages_.size());

    auto size = sizeof(R);
    assert((size % 64) == 0);

    storages_.template emplace_back(size * InitialCapacity, std::byte{ 0 });
    freeRanges_.template emplace_back().insert(std::pair{ size_t{ 0 }, InitialCapacity });
}

template<typename R, size_t N, bool B>
void ResourceManager::registerResource(resource_reg_info_t<R, N, B>)
{
    if constexpr (B) {
        registerFixedSizeResource<R, N>();
    } else {
        registerDynamicSizeResource<R, N>();
    }
}

void ResourceManager::registerResources(ResourceRegInfoSpecialization auto&&... regInfo)
{
    storages_.reserve(sizeof...(regInfo));
    freeRanges_.reserve(sizeof...(regInfo));

    (registerResource(std::forward<decltype(regInfo)>(regInfo)), ...);
}

template<ResourceTypeConcept R, typename... Args>
auto ResourceManager::create(Args&&... args) -> Resource::Id
{
    auto id = allocResource(R::get_tag_const(), sizeof(R));

    auto const [offset, version, storage, _1, _2] = resources_[id.index()];
    (void)_1;
    (void)_2;

    Resource* resource = new (storages_[storage].data() + offset) R(std::forward<Args>(args)...);

    resource->id_ = id;
    resource->resourceManager_ = this;

    return id;
}

// TODO:: create buffers as derived from BaseBuffer derived from Resource
//  (derived buffer just extends base wirth std::array<Size, byte>)
}

#endif // CYCLONITE_RESOURCEMANAGER_H
