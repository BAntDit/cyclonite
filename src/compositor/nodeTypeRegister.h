//
// Created by bantdit on 11/20/22.
//

#ifndef CYCLONITE_NODETYPEREGISTER_H
#define CYCLONITE_NODETYPEREGISTER_H

#include "config.h"

namespace cyclonite::compositor {
class BaseLogicNode;
class BaseGraphicsNode;

template<NodeConfig Config>
class LogicNode;

template<NodeConfig Config>
class GraphicsNode;

template<NodeConfig Config>
using node_t =
  typename std::conditional_t<config_traits::is_logic_node_v<Config>, LogicNode<Config>, GraphicsNode<Config>>;

template<NodeConfig Config>
using node_base_t =
  typename std::conditional_t<config_traits::is_logic_node_v<Config>, BaseLogicNode, BaseGraphicsNode>;

namespace details {
template<typename... Ts>
struct get_max_node_size;

template<typename T>
struct get_max_node_size<T>
{
    static constexpr size_t value = sizeof(T);
};

template<typename T, typename... Ts>
struct get_max_node_size<T, Ts...>
{
    static constexpr size_t value = std::max(sizeof(T), get_max_node_size<Ts...>::value);
};

template<>
struct get_max_node_size<>
{
    static constexpr size_t value = 0;
};

template<uint64_t Id, typename T, typename... Ts>
struct get_type_id;

template<uint64_t Id, typename T, typename... Ts>
struct get_type_id<Id, T, T, Ts...>
{
    static constexpr uint64_t id_v = Id;
};

template<uint64_t Id, typename T, typename Head, typename... Ts>
struct get_type_id<Id, T, Head, Ts...>
{
    static constexpr uint64_t id_v = get_type_id<Id + 1, T, Ts...>::id_v;
};

template<uint64_t Id, typename T>
struct get_type_id<Id, T>
{
    static constexpr uint64_t id_v = std::numeric_limits<uint64_t>::max();
};

template<uint64_t Id, typename... Ts>
struct get_node_type;

template<typename Head, typename... Ts>
struct get_node_type<0, Head, Ts...>
{
    using type_t = Head;
};

template<uint64_t Id, typename Head, typename... Ts>
struct get_node_type<Id, Head, Ts...>
{
    static_assert(Id < (sizeof...(Ts) + 1));
    using type_t = get_node_type<Id - 1, Ts...>;
};
}

template<typename... NodeTypes>
struct node_type_register
{
    template<NodeConfig Config>
    static constexpr auto get_config_type_id() -> uint64_t
    {
        return details::get_type_id<0, node_t<Config>, NodeTypes...>::id_v;
    }

    template<typename NodeType>
    static constexpr auto get_node_type_id() -> uint64_t
    {
        return details::get_type_id<0, NodeType, NodeTypes...>::id_v;
    }

    template<uint64_t typeId>
    using node_type_t = typename details::get_node_type<typeId, NodeTypes...>::type_t;

    template<uint64_t typeId>
    using node_config_t = typename node_traits::get_node_config<node_type_t<typeId>>;

    template<NodeConfig Config>
    using node_key_t =
      type_pair<node_t<Config>,
                std::integral_constant<uint64_t, details::get_type_id<0, node_t<Config>, NodeTypes...>::id_v>>;

    static constexpr size_t max_node_size_v = details::get_max_node_size<NodeTypes...>::value;
};
}

#endif // CYCLONITE_NODETYPEREGISTER_H
