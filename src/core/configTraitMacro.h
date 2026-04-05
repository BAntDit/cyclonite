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

#define DECLARE_CONFIG_TYPE_TRAIT(type_trait, default_type)                                                            \
    template<typename C, typename Enable = void>                                                                       \
    struct has_##type_trait : std::false_type                                                                          \
    {};                                                                                                                \
    template<typename C>                                                                                               \
    struct has_##type_trait<C, std::void_t<typename C::type_trait>> : std::true_type                                   \
    {};                                                                                                                \
    using type_trait = std::conditional_t<has_##type_trait<T>::value, typename T::type_trait, default_type>;

#define DEFINE_CONFIG_TYPE_TRAIT(name) using name##_t = typename internal::config_traits_declaration<Config>::name;

#endif // CYCLONITE_CORE_CONFIG_TRAITS_H
