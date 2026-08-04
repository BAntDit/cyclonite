//
// Created by anton on 8/4/26.
//

#ifndef CYCLONITE_ENTITY_MANAGER_H
#define CYCLONITE_ENTITY_MANAGER_H

#include "entity.h"
#include "enttx/configTraits.h"
#include <vector>

namespace cyclonite::enttx
{
template<typename Config>
class EntityManager
{
private:
    template<typename EnttManagerConfig>
    struct Meta
    {
        // TODO::
    };
public:
    using config_t = enttx::internal::ConfigTraits<Config>;

    using meta_t = Meta<config_t>;

private:
    std::vector<uint32_t> versions_;
};
}

#endif //CYCLONITE_ENTITY_MANAGER_H