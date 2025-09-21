//
// Created by anton on 9/21/25.
//

#ifndef CYCLONITE_REF_FROM_THIS_MIXIN_H
#define CYCLONITE_REF_FROM_THIS_MIXIN_H

#include "resourceBase.h"
#include "resourceSharedRef.h"
#include "resourceWeakRef.h"

namespace cyclonite::core {
class EnableRefFromThis
{
public:
    EnableRefFromThis() = default;

protected:
    template<typename T>
        requires std::is_base_of_v<ResourceBase, T>
    static auto getWeakFromThis(T* t) -> ResourceWeakRef;

    template<typename T>
        requires std::is_base_of_v<ResourceBase, T>
    static auto getSharedFromThis(T* t) -> ResourceSharedRef;
};

template<typename T>
    requires std::is_base_of_v<ResourceBase, T>
auto EnableRefFromThis::getWeakFromThis(T* t) -> ResourceWeakRef
{
    return ResourceWeakRef{ t };
}

template<typename T>
    requires std::is_base_of_v<ResourceBase, T>
auto EnableRefFromThis::getSharedFromThis(T* t) -> ResourceSharedRef
{
    return ResourceSharedRef{ t };
}
}

#endif // CYCLONITE_REF_FROM_THIS_MIXIN_H