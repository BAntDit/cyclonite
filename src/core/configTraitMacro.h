//
// Created by anton on 8/9/25.
//

#ifndef CYCLONITE_CORE_CONFIG_TRAITS_H
#define CYCLONITE_CORE_CONFIG_TRAITS_H

#include <type_traits>

#define DECLARE_CONFIG_TRAIT(name, type, default_value)                                                                \
    template<typename C>                                                                                               \
    static constexpr auto test_##name(decltype(&C::name))->yes_t;                                                      \
    template<typename C>                                                                                               \
    static constexpr auto test_##name(...)->no_t;                                                                      \
    static constexpr auto has_##name = sizeof(test_##name<T>(0)) == sizeof(yes_t);                                     \
    static constexpr auto name() -> type                                                                               \
    {                                                                                                                  \
        if constexpr (has_##name) {                                                                                    \
            static_assert(std::is_convertible_v<std::decay_t<decltype(T::name)>, type>);                               \
            return T::name;                                                                                            \
        } else {                                                                                                       \
            return type{ default_value };                                                                              \
        }                                                                                                              \
    }

#define DEFINE_CONFIG_TRAIT(name) static constexpr auto name##_v = internal::config_traits_declaration<Config>::name();

#endif // CYCLONITE_CORE_CONFIG_TRAITS_H
