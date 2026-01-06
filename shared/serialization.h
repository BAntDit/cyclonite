//
// Created by anton on 12/29/25.
//

#ifndef CYCLONITE_SHARED_SERIALIZATION_H
#define CYCLONITE_SHARED_SERIALIZATION_H

#include <array>
#include <concepts>
#include <tuple>
#include <type_traits>

// serialization based on https://github.com/mera-company/cpp-serialization-library/tree/master
namespace cyclonite::shared {
template<typename T>
concept SerializationDataWriterConcept = std::invocable<T>;

namespace internal {
template<auto... Values>
struct value_list
{};
}

template<typename T, SerializationDataWriterConcept SerializationWriter>
class SerializationInvoker
{
public:
    template<auto... Accessor>
    explicit constexpr SerializationInvoker(internal::value_list<Accessor...>)
      : invokePtr_{ &serializationInvoke<Accessor...> }
    {
    }

    constexpr void operator()(T& t, SerializationWriter& sw) { (*invokePtr_)(t, sw); }

private:
    template<auto... Accessor>
    static constexpr void serializationInvoke(T& t, SerializationWriter& si);

    using serialization_invoke_ptr_t = void (*)(T&, SerializationWriter&);

    serialization_invoke_ptr_t invokePtr_;
};

namespace internal {
template<typename Ret, typename T, typename... Args>
struct accessor_method_info_t
{
    using return_type_t = Ret;
    using class_type_t = T;
    using arguments_t = std::tuple<Args...>;
    using decayed_arguments_t = std::tuple<std::remove_pointer_t<std::decay_t<Args>>...>;
};

template<typename>
struct accessor_info_t;

// all overloads, but no volatile
// (volatile can be added later, if necessary)
template<typename Ret, typename T, typename... Args>
struct accessor_info_t<Ret (T::*)(Args...)> : accessor_method_info_t<Ret, T, Args...>
{};

template<typename Ret, typename Class, typename... Args>
struct accessor_info_t<Ret (Class::*)(Args...) noexcept> : accessor_method_info_t<Ret, Class, Args...>
{};

template<typename Ret, typename Class, typename... Args>
struct accessor_info_t<Ret (Class::*)(Args...)&> : accessor_method_info_t<Ret, Class, Args...>
{};

template<typename Ret, typename Class, typename... Args>
struct accessor_info_t<Ret (Class::*)(Args...) & noexcept> : accessor_method_info_t<Ret, Class, Args...>
{};

template<typename Ret, typename Class, typename... Args>
struct accessor_info_t<Ret (Class::*)(Args...) &&> : accessor_method_info_t<Ret, Class, Args...>
{};

template<typename Ret, typename Class, typename... Args>
struct accessor_info_t<Ret (Class::*)(Args...) && noexcept> : accessor_method_info_t<Ret, Class, Args...>
{};

template<typename Ret, typename Class, typename... Args>
struct accessor_info_t<Ret (Class::*)(Args...) const> : accessor_method_info_t<Ret, Class, Args...>
{};

template<typename Ret, typename Class, typename... Args>
struct accessor_info_t<Ret (Class::*)(Args...) const noexcept> : accessor_method_info_t<Ret, Class, Args...>
{};

template<typename Ret, typename Class, typename... Args>
struct accessor_info_t<Ret (Class::*)(Args...) const&> : accessor_method_info_t<Ret, Class, Args...>
{};

template<typename Ret, typename Class, typename... Args>
struct accessor_info_t<Ret (Class::*)(Args...) const & noexcept> : accessor_method_info_t<Ret, Class, Args...>
{};

template<typename Ret, typename Class, typename... Args>
struct accessor_info_t<Ret (Class::*)(Args...) const && noexcept> : accessor_method_info_t<Ret, Class, Args...>
{};

template<typename... Accessors>
struct first_accessor_class_type;

template<typename First, typename... Rest>
struct first_accessor_class_type<First, Rest...>
{
    using type = typename accessor_info_t<First>::class_type_t;
};

template<typename... Accessors>
using first_accessor_class_type_t = typename first_accessor_class_type<Accessors...>::type;

template<typename Type, typename PassedType>
inline constexpr auto passArgument(PassedType&& val) -> decltype(auto)
{
    if constexpr (std::is_pointer_v<Type>) {
        return std::addressof(val);
    } else {
        return std::forward<PassedType>(val);
    }
}

template<typename Accessor>
class SerializationChainItem
{
public:
    using decayed_args_tuple_t = typename accessor_info_t<Accessor>::decayed_arguments_t;
    using args_tuple_t = typename accessor_info_t<Accessor>::arguments_t;
    using class_t = typename accessor_info_t<Accessor>::class_type_t;

    static constexpr size_t args_count_v = std::tuple_size_v<args_tuple_t>;

    template<typename T>
    constexpr SerializationChainItem(Accessor const& accessor, T& t);

    args_tuple_t arguments;

    template<typename NextAccessor>
    constexpr auto operator<<(NextAccessor const& nextAccessor) && -> SerializationChainItem<NextAccessor>
    {
        using invoking_class_t = typename accessor_info_t<NextAccessor>::class_type_t;
        return SerializationChainItem<NextAccessor>{ nextAccessor, std::get<invoking_class_t>(arguments) };
    }

private:
    template<typename T, size_t... Idx>
    constexpr void invokeImpl(Accessor const& accessor, T& t, std::index_sequence<Idx...>);
};

template<typename Accessor>
template<typename T, size_t... Idx>
constexpr void SerializationChainItem<Accessor>::invokeImpl(Accessor const& accessor, T& t, std::index_sequence<Idx...>)
{
    (t.*accessor)(passArgument<std::tuple_element_t<Idx, decayed_args_tuple_t>>(std::get<Idx>(arguments))...);
}

template<typename Accessor>
template<typename T>
constexpr SerializationChainItem<Accessor>::SerializationChainItem(Accessor const& accessor, T& t)
{
    invokeImpl(accessor, t, std::make_index_sequence<args_count_v>{});
}

template<typename T>
struct SerializationChainBeginner
{
    explicit constexpr SerializationChainBeginner(T& t)
      : t_(t)
    {
    }

    template<typename NextAccessor>
    constexpr auto operator<<(NextAccessor const& nextAccessor) -> SerializationChainItem<NextAccessor>
    {
        return SerializationChainItem<NextAccessor>{ nextAccessor, t_ };
    }

private:
    T& t_;
};

template<typename T, typename... Accessor>
constexpr auto invokeSerializationChain(T&& t, Accessor&&... accessor)
{
    return (SerializationChainBeginner<std::decay_t<T>>{ t } << ... << std::forward<Accessor>(accessor)).arguments;
}

template<SerializationDataWriterConcept SerializationWriter>
struct serialization_interface_wrap_t
{};

template<auto... Accessor>
struct SerilalizationInvokeForwarder
{
    template<SerializationDataWriterConcept SerializationWriter>
    constexpr auto serializationInvoker() const noexcept
    {
        return SerializationInvoker<first_accessor_class_type<decltype(Accessor)...>, SerializationWriter>{
            value_list<Accessor...>{}
        };
    }
};
}

template<SerializationDataWriterConcept SerializationWriter, typename... Args>
constexpr inline auto useWriter(Args&&... args) -> internal::serialization_interface_wrap_t<SerializationWriter>
{
    return internal::serialization_interface_wrap_t<SerializationWriter>(std::forward<Args>(args)...);
}

template<typename T, SerializationDataWriterConcept SerializationWriter>
template<auto... Accessor>
/*static*/ constexpr void SerializationInvoker<T, SerializationWriter>::serializationInvoke(T& t,
                                                                                            SerializationWriter& sw)
{
    sw(internal::invokeSerializationChain(t, Accessor...));
}

template<auto... Accessor>
constexpr auto makeSerialzationDataAccessChain()
{
    return internal::SerilalizationInvokeForwarder<Accessor...>{};
}

template<typename T, size_t N, SerializationDataWriterConcept SerializationWriter>
class Serializer
{
public:
    using data_access_chain_t = SerializationInvoker<T, SerializationWriter>;

    template<typename... AccessChain>
    explicit constexpr Serializer(internal::serialization_interface_wrap_t<SerializationWriter>,
                                  AccessChain&&... accessChain);

    constexpr void operator()(T& t, SerializationWriter& si) const;

private:
    std::array<data_access_chain_t, N> accessChains_;
};

template<typename T, size_t N, SerializationDataWriterConcept SerializationWriter>
template<typename... AccessChain>
constexpr Serializer<T, N, SerializationWriter>::Serializer(
  internal::serialization_interface_wrap_t<SerializationWriter>,
  AccessChain&&... accessChain)
  : accessChains_{ accessChain.template serializationInvoker<SerializationWriter>()... }
{
}

template<typename T, size_t N, SerializationDataWriterConcept SerializationWriter>
constexpr void Serializer<T, N, SerializationWriter>::operator()(T& t, SerializationWriter& sw) const
{
    for (auto const& accessChainInvoke : accessChains_) {
        accessChainInvoke(t, sw);
    }
}

// deduction guide
template<SerializationDataWriterConcept SerializationWriter, typename... AccessChain>
explicit Serializer(internal::serialization_interface_wrap_t<SerializationWriter>, AccessChain...)
  -> Serializer<internal::first_accessor_class_type_t<AccessChain...>, sizeof...(AccessChain), SerializationWriter>;
}

#endif // CYCLONITE_SHARED_SERIALIZATION_H