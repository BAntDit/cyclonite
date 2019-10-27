//
// Created by bantdit on 1/27/19.
//

#ifndef CYCLONITE_READOPTIONAL_H
#define CYCLONITE_READOPTIONAL_H

#include <nlohmann/json.hpp>

namespace cyclonite::loaders::gltf::internal {
template<typename T>
static auto getOptional(nlohmann::json& json, std::string const& property, T&& defaultValue) -> std::decay_t<T>
{
    typename std::decay<T>::type result;

    auto it = json.find(property);

    if (it != json.end()) {
        try {
            result = it.value().get<T>();
        } catch (...) {
            result = std::forward<T>(defaultValue);
        }
    } else {
        result = std::forward<T>(defaultValue);
    }

    return result;
}
}

#endif // CYCLONITE_READOPTIONAL_H
