//
// Created by anton on 10/28/26.
//

#ifndef CYCLONITE_RESOURCES_RESOURCE_GROUP_MANAGER_H
#define CYCLONITE_RESOURCES_RESOURCE_GROUP_MANAGER_H

#include "resourceGroup.h"
#include "resourceGroupManagerBase.h"
#include <atomic>
#include <bitset>

namespace cyclonite::resources {
template<typename Config>
class ResourceGroupManager : public ResourceGroupManagerBase
{
    using resource_type_list_t = typename Config::resource_type_list_t;

    static constexpr size_t resource_type_list_size_v = resource_type_list_t::size;
    static_assert(resource_type_list_size_v <= 64, "mask is stored in uint64_t");

    template<uint64_t Mask, typename IndexSequence>
    struct mask_to_type_list;

    template<uint64_t Mask, size_t Last>
    struct mask_to_type_list<Mask, std::index_sequence<Last>>
    {
        using type_at_bit_t = resource_type_list_t::template get_type<Last>::type;

        using type_list_at_bit_t =
          std::conditional_t<((Mask >> Last) & 1), metrix::type_list<type_at_bit_t>, metrix::type_list<>>;

        using type = type_list_at_bit_t;
    };

    template<uint64_t Mask, size_t First, size_t... I>
    struct mask_to_type_list<Mask, std::index_sequence<First, I...>>
    {
        using type_at_bit_t = resource_type_list_t::template get_type<First>::type;

        using type_list_at_bit_t =
          std::conditional_t<((Mask >> First) & 1), metrix::type_list<type_at_bit_t>, metrix::type_list<>>;

        using type =
          metrix::concat<type_list_at_bit_t, typename mask_to_type_list<Mask, std::index_sequence<I...>>::type>::type;
    };

    template<uint64_t Mask>
    struct to_group_type
    {
        using group_type_list = mask_to_type_list<Mask, std::make_index_sequence<resource_type_list_size_v>>::type;
        using group_type = group_type_list::template specialization_t<ResourceGroup>;
    };

    template<uint64_t Mask>
    using to_group_type_t = to_group_type<Mask>::group_type;

    template<typename... T>
    constexpr static auto type_list_to_mask(metrix::type_list<T...>) -> std::bitset<resource_type_list_size_v>
    {
        static_assert((resource_type_list_t::template has_type<T>::value && ...));

        auto r = std::bitset<resource_type_list_size_v>();

        (r.set(resource_type_list_t::template get_type_index<T>::value), ...);

        return r;
    }

public:
    ResourceGroupManager() = default;

    template<typename... ResourceType>
    auto addGroup(std::string_view name) -> uint32_t;

    void deleteGroup(std::string_view name);

    template<typename OnVisit, typename... Args>
    auto visitGroupAsync(uint32_t groupId, OnVisit&& onVisit, Args&&... args) -> std::future<void>
    {
        assert(groupAndMaskPairs_.contains(groupId));
        auto&& [baseGroupPtr, groupMask] = groupAndMaskPairs_[groupId];

        return visitGroupAsyncImpl<0, metrix::type_list<>>(
          groupMask, *baseGroupPtr, std::forward<OnVisit>(onVisit), std::forward<Args>(args)...);
    }

    template<typename OnVisit, typename... Args>
    auto visitGroupAsync(std::string_view groupName, OnVisit&& onVisit, Args&&... args) -> std::future<void>
    {
        assert(groupNameToId_.contains(std::string{ groupName }));
        auto id = groupNameToId_[std::string{ groupName }];

        return visitGroupAsync(id, std::forward<OnVisit>(onVisit), std::forward<Args>(args)...);
    }

    template<typename OnVisit, typename... Args>
    void visitGroup(std::string_view groupName, OnVisit&& onVisit, Args&&... args)
    {
        assert(groupNameToId_.contains(std::string{ groupName }));
        auto id = groupNameToId_[std::string{ groupName }];

        visitGroup(id, std::forward<OnVisit>(onVisit), std::forward<Args>(args)...);
    }

    template<typename OnVisit, typename... Args>
    void visitGroup(uint32_t groupId, OnVisit&& onVisit, Args&&... args)
    {
        assert(groupAndMaskPairs_.contains(groupId));
        auto&& [baseGroupPtr, groupMask] = groupAndMaskPairs_[groupId];

        visitGroupImpl<0, metrix::type_list<>>(
          groupMask.to_ullong(), *baseGroupPtr, std::forward<OnVisit>(onVisit), std::forward<Args>(args)...);
    }

private:
    template<size_t CurrentBit, typename AccumulatedTypeList, typename OnVisit, typename... Args>
    static void visitGroupImpl(uint64_t actualMask, ResourceGroupBase& base, OnVisit&& onVisit, Args&&... args)
    {
        if constexpr (CurrentBit == resource_type_list_size_v) {
            // All bits resolved.
            using group_t = typename AccumulatedTypeList::template specialization_t<ResourceGroup>;
            onVisit(static_cast<group_t&>(base), std::forward<Args>(args)...);
        } else {
            using type_at_bit_t = typename resource_type_list_t::template get_type<CurrentBit>::type;

            if ((actualMask >> CurrentBit) & 1) {
                using next_type_list_t =
                  typename metrix::concat<AccumulatedTypeList, metrix::type_list<type_at_bit_t>>::type;

                visitGroupImpl<CurrentBit + 1, next_type_list_t>(
                  actualMask, base, std::forward<OnVisit>(onVisit), std::forward<Args>(args)...);
            } else {
                visitGroupImpl<CurrentBit + 1, AccumulatedTypeList>(
                  actualMask, base, std::forward<OnVisit>(onVisit), std::forward<Args>(args)...);
            }
        }
    }

    template<size_t CurrentBit, typename AccumulatedTypeList, typename OnVisit, typename... Args>
    static auto visitGroupAsyncImpl(uint64_t actualMask, ResourceGroupBase& base, OnVisit&& onVisit, Args&&... args)
      -> std::future<void>
    {
        auto result = std::future<void>{};

        if constexpr (CurrentBit == resource_type_list_size_v) {
            // All bits resolved.
            using group_t = typename AccumulatedTypeList::template specialization_t<ResourceGroup>;
            result = onVisit(static_cast<group_t&>(base), std::forward<Args>(args)...);
        } else {
            using type_at_bit_t = typename resource_type_list_t::template get_type<CurrentBit>::type;

            if ((actualMask >> CurrentBit) & 1) {
                using next_type_list_t =
                  typename metrix::concat<AccumulatedTypeList, metrix::type_list<type_at_bit_t>>::type;

                result = visitGroupAsyncImpl<CurrentBit + 1, next_type_list_t>(
                  actualMask, base, std::forward<OnVisit>(onVisit), std::forward<Args>(args)...);
            } else {
                result = visitGroupAsyncImpl<CurrentBit + 1, AccumulatedTypeList>(
                  actualMask, base, std::forward<OnVisit>(onVisit), std::forward<Args>(args)...);
            }
        }

        return result;
    }

    static std::atomic<uint32_t> lastGroupId;

    std::unordered_map<uint32_t, std::pair<std::unique_ptr<ResourceGroupBase>, std::bitset<resource_type_list_size_v>>>
      groupAndMaskPairs_;
    std::unordered_map<std::string, uint32_t> groupNameToId_;
};

template<typename Config>
std::atomic<uint32_t> ResourceGroupManager<Config>::lastGroupId{ 1 };

template<typename Config>
template<typename... ResourceType>
auto ResourceGroupManager<Config>::addGroup(std::string_view name) -> uint32_t
{
    auto id = lastGroupId.fetch_add(1, std::memory_order_acq_rel);

    if (groupNameToId_.contains(std::string{ name })) {
        throw std::invalid_argument("each group must have unique name");
    }
    groupNameToId_.emplace(std::string{ name }, id);

    auto group = std::unique_ptr<ResourceGroupBase>(new ResourceGroup<ResourceType...>{ *this, id });
    auto mask = type_list_to_mask(metrix::type_list<ResourceType...>{});

    groupAndMaskPairs_.emplace(id, std::pair{ std::move(group), mask });

    return id;
}

template<typename Config>
void ResourceGroupManager<Config>::deleteGroup(std::string_view name)
{
    assert(groupNameToId_.contains(std::string{ name }));
    auto id = groupNameToId_[std::string{ name }];
    groupNameToId_.erase(std::string{ name });

    assert(groupAndMaskPairs_.contains(id));
    groupAndMaskPairs_.erase(id);
}
}
#endif // CYCLONITE_RESOURCES_RESOURCE_GROUP_MANAGER_H
