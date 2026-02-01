//
// Created by anton on 12/29/25.
//

#ifndef CYCLONITE_SHARED_SERIALIZATION_H
#define CYCLONITE_SHARED_SERIALIZATION_H

#include <array>
#include <concepts>
#include <cstddef>
#include <cstdint>
#include <metrix/containers.h>
#include <metrix/type_traits.h>
#include <tuple>
#include <type_traits>
#include <unistd.h>
#include <vector>

namespace cyclonite::shared {
namespace internal {
template<typename StreamWriter>
struct stream_writer_type_wrap_t
{};

template<typename... Accessors>
struct first_accessor_class_type;

template<typename First, typename... Rest>
struct first_accessor_class_type<First, Rest...>
{
    using type = typename metrix::member_function_class_type_t<First>;
};

template<typename... Accessors>
using first_accessor_class_type_t = typename first_accessor_class_type<Accessors...>::type;

template<auto... Values>
struct value_list
{};

template<typename ArgumentsList>
struct args_tuple;

template<typename... Args>
struct args_tuple<metrix::type_list<Args...>>
{
    using type = std::tuple<std::remove_pointer_t<std::decay_t<Args>>...>;
};

template<typename ArgumentsList>
using args_tuple_t = typename args_tuple<ArgumentsList>::type;

template<typename ArgumentsList>
struct decayed_args_tuple;

template<typename... Args>
struct decayed_args_tuple<metrix::type_list<Args...>>
{
    using type = std::tuple<Args...>;
};

template<typename ArgumentsList>
using decayed_args_tuple_t = typename decayed_args_tuple<ArgumentsList>::type;

template<typename TupleType, typename T>
struct has_element;

template<typename... Element, typename T>
struct has_element<std::tuple<Element...>, T>
{
    static constexpr bool value = metrix::type_list<Element...>::template has_type<T>::value;
};

template<typename TupleType, typename T>
inline constexpr bool has_element_v = has_element<TupleType, T>::value;

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
class AccessChainItem
{
public:
    using class_t = metrix::member_function_class_type_t<Accessor>;
    using args_tuple_t = args_tuple_t<metrix::member_function_argument_type_list_t<Accessor>>;
    using decayed_args_tuple_t = decayed_args_tuple_t<metrix::member_function_argument_type_list_t<Accessor>>;

    static constexpr size_t args_count_v = std::tuple_size_v<args_tuple_t>;

    decayed_args_tuple_t arguments;

    template<typename AnyObject>
    AccessChainItem(Accessor const& accessor, AnyObject& anyObject);

    template<typename NextAccessor>
    auto operator<<(NextAccessor& nextAccessor) &&;

    auto getArguments()
    {
        using args_vector_t = std::vector<typename AccessChainItem<Accessor>::decayed_args_tuple_t>;

        auto res = args_vector_t{};
        res.emplace_back(arguments);

        return res;
    }

private:
    template<typename AnyObject, size_t... Idx>
    void invokeImpl(Accessor const& accessor, AnyObject& anyObject, std::index_sequence<Idx...>);
};

template<typename Accessor>
class AccessChainItemArray
{
public:
    AccessChainItemArray() = default;

    template<typename AnyObject>
    void addAccessChainItem(Accessor const& accessor, AnyObject& anyObject);

    template<typename NextAccessor>
    auto operator<<(NextAccessor& nextAccessor) && -> AccessChainItemArray<NextAccessor>;

    auto getArguments()
    {
        using args_vector_t = std::vector<typename AccessChainItem<Accessor>::decayed_args_tuple_t>;

        auto res = args_vector_t{};
        res.reserve(accessChainItems_.size());

        for (auto& accessChainItem : accessChainItems_) {
            res.emplace_back(accessChainItem.arguments);
        }

        return res;
    }

private:
    std::vector<AccessChainItem<Accessor>> accessChainItems_;
};

template<typename Accessor>
template<typename AnyObject>
void AccessChainItemArray<Accessor>::addAccessChainItem(Accessor const& accessor, AnyObject& anyObject)
{
    accessChainItems_.emplace_back(accessor, anyObject);
}

template<typename Accessor>
template<typename NextAccessor>
auto AccessChainItemArray<Accessor>::operator<<(NextAccessor& nextAccessor) && -> AccessChainItemArray<NextAccessor>
{
    using invoking_class_t = typename metrix::member_function_class_type_t<NextAccessor>;

    auto accessChainItemArray = AccessChainItemArray<NextAccessor>{};
    for (auto& accessChainItem : accessChainItems_) {
        accessChainItemArray.addAccessChainItem(nextAccessor, std::get<invoking_class_t>(accessChainItem.arguments));
    }

    return accessChainItemArray;
}

// AccessChainItem methods:
template<typename Accessor>
template<typename AnyObject>
AccessChainItem<Accessor>::AccessChainItem(Accessor const& accessor, AnyObject& anyObject)
  : arguments{}
{
    invokeImpl(accessor, anyObject, arguments, std::make_index_sequence<args_count_v>{});
}

template<typename Accessor>
template<typename AnyObject, size_t... Idx>
void AccessChainItem<Accessor>::invokeImpl(Accessor const& accessor, AnyObject& anyObject, std::index_sequence<Idx...>)
{
    (anyObject.*accessor)(passArgument<std::tuple_element_t<Idx, args_tuple_t>>(std::get<Idx>(arguments))...);
}

template<typename Accessor>
template<typename NextAccessor>
auto AccessChainItem<Accessor>::operator<<(NextAccessor& nextAccessor) &&
{
    using invoking_class_t = typename metrix::member_function_class_type_t<NextAccessor>;

    if constexpr (has_element_v<decayed_args_tuple_t, invoking_class_t>) {
        return AccessChainItem<NextAccessor>(nextAccessor, std::get<invoking_class_t>(arguments));
    } else if constexpr (has_element_v<decayed_args_tuple_t, std::vector<invoking_class_t>>) {
        auto accessChainItemArray = AccessChainItemArray<NextAccessor>{};
        auto& objectArray = std::get<std::vector<invoking_class_t>>(arguments);
        for (auto& anyObject : objectArray) {
            accessChainItemArray.addAccessChainItem(nextAccessor, anyObject);
        }
        return accessChainItemArray;
    } /*else if extends for other types here*/
}
//

template<typename AnyObject>
class AccessChainBeginner
{
public:
    explicit AccessChainBeginner(AnyObject& anyObject)
      : anyObject_{ anyObject }
    {
    }

    template<typename NextAccessor>
    auto operator<<(NextAccessor const& nextAccessor) -> AccessChainItem<NextAccessor>
    {
        return AccessChainItem<NextAccessor>{ anyObject_, nextAccessor };
    }

private:
    AnyObject& anyObject_;
};

template<typename AnyObject, typename... Accessor>
auto invokeAccessChain(AnyObject&& anyObject, Accessor&&... accessor)
{
    return (AccessChainBeginner<std::decay_t<AnyObject>>{ anyObject } << ... << std::forward<Accessor>(accessor))
      .getArguments();
}

template<typename AnyObject, typename StreamWriter>
class AccessChainInvoker
{
public:
    template<auto... Accessor>
    explicit AccessChainInvoker(value_list<Accessor...>);

    void operator()(AnyObject& anyObject, StreamWriter& streamWriter) const;

private:
    template<auto... Accessor>
    static void accessChainInvoke(AnyObject& anyObject, StreamWriter& streamWriter);

private:
    using access_chain_invoker_f = void (*)(AnyObject&, StreamWriter&);

    access_chain_invoker_f invoke_;
};

template<typename AnyObject, typename StreamWriter>
template<auto... Accessor>
/*explicit*/ AccessChainInvoker<AnyObject, StreamWriter>::AccessChainInvoker(value_list<Accessor...>)
  : invoke_{ &accessChainInvoke<Accessor...> }
{
}

template<typename AnyObject, typename StreamWriter>
template<auto... Accessor>
/*static*/ void AccessChainInvoker<AnyObject, StreamWriter>::accessChainInvoke(AnyObject& anyObject,
                                                                               StreamWriter& streamWriter)
{
    auto&& argsVec = invokeAccessChain(anyObject, Accessor...);
    for (auto&& args : argsVec) {
        streamWriter << args;
    }
}

template<typename AnyObject, typename StreamWriter>
void AccessChainInvoker<AnyObject, StreamWriter>::operator()(AnyObject& anyObject, StreamWriter& streamWriter) const
{
    invoke_(anyObject, streamWriter);
}

template<auto... Accessor>
struct AccessChainInvokeForwarder
{
    template<typename StreamWriter>
    auto invoke() const {
        using object_type_t = typename metrix::type_list<decltype(Accessor)...>::template get_type<0>::type;
        return AccessChainInvoker<object_type_t, StreamWriter>{ value_list<Accessor...>{} };
    }
};
}

template<auto... Accessor>
constexpr auto makeAccessChain()
{
    return internal::AccessChainInvokeForwarder<Accessor...>{};
}

template<typename AnyObject, size_t N, typename StreamWriter>
class Serializer
{
public:
    using data_access_chain_t = internal::AccessChainInvoker<AnyObject, StreamWriter>;

    template<typename... AccessChain>
    constexpr Serializer(internal::stream_writer_type_wrap_t<StreamWriter>, AccessChain&&... accessChain); // TODO::

    void operator()(AnyObject& anyObject, StreamWriter& sw) const; // TODO:: ...

private:
    std::array<data_access_chain_t, N> accessChains_;
};

template<typename StreamWriter>
constexpr inline auto useWriter() -> internal::stream_writer_type_wrap_t<StreamWriter>
{
    return internal::stream_writer_type_wrap_t<StreamWriter>{};
}

// deduction guide:
template<typename StreamWriter, typename... AccessChain>
Serializer(internal::stream_writer_type_wrap_t<StreamWriter>, AccessChain&&... accessChain)
  -> Serializer<internal::first_accessor_class_type_t<AccessChain...>, sizeof...(AccessChain), StreamWriter>;
}

#endif // CYCLONITE_SHARED_SERIALIZATION_H