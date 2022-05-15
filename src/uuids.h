//
// Created by anton on 11/7/21.
//

#ifndef CYCLONITE_UUIDS_H
#define CYCLONITE_UUIDS_H

#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <mutex>

namespace cyclonite {
inline auto getNewUniqueUUID() -> boost::uuids::uuid
{
    static auto generator = boost::uuids::random_generator{};
    static auto mutex = std::recursive_mutex{};

    auto lock = std::lock_guard<std::recursive_mutex>{ mutex };

    return generator();
}

inline auto getNilUUID() -> boost::uuids::uuid
{
    return boost::uuids::nil_uuid();
}
}

#endif // CYCLONITE_UUIDS_H
