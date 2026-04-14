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
        default:
            assert(false); // unexpected state
    }
}

void ResourceGroupBase::notifyResourceAdded(std::string_view resourceName)
{
    groupManager_->resourceAdded(id_, resourceName);
}
}
