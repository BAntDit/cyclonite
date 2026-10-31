//
// Created by anton on 4/13/26.
//

#include "resourceGroupBase.h"

namespace cyclonite::resources {
void ResourceGroupBase::notifyResourceStateChange(ManagedResourceState newState, std::string_view resourceName)
{
    switch (newState) {
        case ManagedResourceState::Loading:
            groupManager_->resourceLoadingStart(id_, resourceName);
            break;
        case ManagedResourceState::Loaded:
            groupManager_->resourceLoaded(id_, resourceName);
            break;
        case ManagedResourceState::Preparing:
            groupManager_->resourcePreparingStart(id_, resourceName);
            break;
        case ManagedResourceState::GoingToBeRemoved:
            groupManager_->resourceGoingToBeDeleted(id_, resourceName);
            break;
        case ManagedResourceState::Ready:
            groupManager_->resourcePrepared(id_, resourceName);
            break;
        default:
            assert(false); // unexpected state
    }
}

void ResourceGroupBase::notifyResourceAdded(std::string_view resourceName)
{
    groupManager_->resourceAdded(id_, resourceName);
}

auto ResourceGroupBase::isExists(boost::uuids::uuid const& uuid) const -> bool
{
    auto us = boost::uuids::to_string(uuid);
    return resourceUuidToRef_.contains(us);
}

auto ResourceGroupBase::isExists(std::string_view name) const -> bool
{
    return resourceNameToRef_.contains(name);
}

auto ResourceGroupBase::getResource(boost::uuids::uuid const& uuid) const -> core::ResourceSharedRef
{
    auto ref = core::ResourceSharedRef{};
    auto us = boost::uuids::to_string(uuid);

    if (resourceUuidToRef_.contains(us)) {
        ref = resourceUuidToRef_.at(us);
    }
    return ref;
}

auto ResourceGroupBase::getResource(std::string_view name) const -> core::ResourceSharedRef
{
    auto ref = core::ResourceSharedRef{};

    if (resourceNameToRef_.contains(name)) {
        ref = resourceNameToRef_.at(name);
    }
    return ref;
}
}
