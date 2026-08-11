//
// Created by anton on 12/29/25.
//

#ifndef CYCLONITE_SHARED_SERIALIZATION_H
#define CYCLONITE_SHARED_SERIALIZATION_H

#include "serializationCommon.h"
#include <ranges>
#include <string>
#include <string_view>
#include <tuple>
#include <type_traits>
#if !defined(_WIN32)
#include <unistd.h>
#endif

namespace cyclonite::shared {
namespace internal {
template<BinaryWriterConcept W, typename T>
    requires std::is_arithmetic_v<T> || std::is_enum_v<T>
void writePrimitive(W& writer, T value)
{
    auto converted = toEndian(value, writer.endianness());
    writer.writeBytes(&converted, sizeof(T));
}

template<BinaryReaderConcept R, typename T>
    requires std::is_arithmetic_v<T> || std::is_enum_v<T>
auto readPrimitive(R& reader, T& value) -> void
{
    auto raw = T{};
    reader.readBytes(&raw, sizeof(T));
    value = fromEndian(raw, reader.endianness());
}

using serialized_container_length_type_t = uint32_t;

template<typename T>
struct is_stringlike : std::false_type
{};

template<>
struct is_stringlike<std::string> : std::true_type
{};

template<>
struct is_stringlike<std::string_view> : std::true_type
{};

template<typename T>
inline constexpr bool is_stringlike_v = is_stringlike<std::decay_t<T>>::value;
}

template<BinaryWriterConcept W, typename T>
void serializeValue(W& writer, T const& value);

template<BinaryReaderConcept R, typename T>
void deserializeValue(R& reader, T& value);

template<BinaryWriterConcept W, typename T>
void serializeValue(W& writer, T const& value)
{
    using decayed_t = std::decay_t<T>;

    if constexpr (internal::is_stringlike_v<decayed_t>) {
        auto length = static_cast<internal::serialized_container_length_type_t>(value.size());
        internal::writePrimitive(writer, length);

        if (length > 0)
            writer.writeBytes(value.data(), length);
    } else if constexpr (std::ranges::contiguous_range<decayed_t>) {
        auto length = static_cast<internal::serialized_container_length_type_t>(value.size());
        internal::writePrimitive(writer, length);

        for (auto const& element : value)
            serializeValue(writer, element);
    } else if constexpr (std::is_arithmetic_v<decayed_t> || std::is_enum_v<decayed_t>) {
        internal::writePrimitive(writer, value);
    } else {
        static_assert(!sizeof(T), "serializeValue: unsupported value type - add an overload / trait specialization");
    }
}

template<BinaryReaderConcept R, typename T>
auto deserializeValue(R& reader, T& value) -> void
{
    using decayed_t = std::decay_t<T>;

    if constexpr (std::is_same_v<decayed_t, std::string>) {
        auto length = internal::serialized_container_length_type_t{};
        internal::readPrimitive(reader, length);
        value.resize(length);

        if (length > 0)
            reader.readBytes(value.data(), length);
    } else if constexpr (std::ranges::contiguous_range<decayed_t>) {
        auto length = internal::serialized_container_length_type_t{};
        internal::readPrimitive(reader, length);
        value.resize(length);

        for (auto& element : value)
            deserializeValue(reader, element);
    } else if constexpr (std::is_arithmetic_v<decayed_t> || std::is_enum_v<decayed_t>) {
        internal::readPrimitive(reader, value);
    } else {
        static_assert(!sizeof(T), "deserializeValue: unsupported value type - add an overload / trait specialization");
    }
}

namespace internal {
template<typename... Args>
inline constexpr bool all_non_const_lvalue_refs_v =
  (... && (std::is_lvalue_reference_v<Args> && !std::is_const_v<std::remove_reference_t<Args>>));

template<typename... Args>
inline constexpr bool all_by_value_or_const_ref_v =
  (... && (!std::is_lvalue_reference_v<Args> || std::is_const_v<std::remove_reference_t<Args>>));

template<typename Ptr>
struct member_fn_info
{
    static constexpr bool is_multi_out_getter_v = false;
    static constexpr bool is_multi_in_setter_v = false;
};

template<typename C, typename... Args>
struct member_fn_info<void (C::*)(Args...) const>
{
    static constexpr bool is_multi_out_getter_v = sizeof...(Args) > 0 && all_non_const_lvalue_refs_v<Args...>;
    static constexpr bool is_multi_in_setter_v = false;

    template<auto Fn, BinaryWriterConcept W>
    static void writeAll(W& writer, C const& obj)
    {
        auto values = std::tuple<std::remove_cvref_t<Args>...>{};
        std::apply([&](auto&... vals) { (obj.*Fn)(vals...); }, values);
        std::apply([&](auto const&... vals) { (serializeValue(writer, vals), ...); }, values);
    }
};

template<typename C, typename... Args>
struct member_fn_info<void (C::*)(Args...)>
{
    static constexpr bool is_multi_out_getter_v = sizeof...(Args) > 0 && all_non_const_lvalue_refs_v<Args...>;
    static constexpr bool is_multi_in_setter_v = sizeof...(Args) > 0 && all_by_value_or_const_ref_v<Args...>;

    template<auto Fn, BinaryWriterConcept W>
    static void writeAll(W& writer, C const& obj)
    {
        auto values = std::tuple<std::remove_cvref_t<Args>...>{};
        std::apply([&](auto&... vals) { (obj.*Fn)(vals...); }, values);
        std::apply([&](auto const&... vals) { (serializeValue(writer, vals), ...); }, values);
    }

    template<auto Fn, BinaryReaderConcept R>
    static auto readAll(R& reader, C& obj) -> void
    {
        auto values = std::tuple<std::remove_cvref_t<Args>...>{};
        std::apply([&](auto&... vals) { (deserializeValue(reader, vals), ...); }, values);
        std::apply([&](auto const&... vals) { (obj.*Fn)(vals...); }, values);
    }
};
}

// Compile-time access chains
namespace internal {
template<auto Head, auto... Tail>
struct AccessChainResolver
{
    using access_ptr_type_t = decltype(Head);

    template<BinaryWriterConcept W, typename T>
    static void serialize(W& writer, T const& obj)
    {
        if constexpr (std::is_member_function_pointer_v<access_ptr_type_t>) {
            using info_t = member_fn_info<access_ptr_type_t>;

            if constexpr (info_t::is_multi_out_getter_v) {
                static_assert(
                  sizeof...(Tail) == 0,
                  "AccessChain: a multi-output getter (void(T&...) const) must be the last access chain item");
                info_t::template writeAll<Head>(writer, obj);
            } else {
                auto&& result = (obj.*Head)();
                serializeNext(writer, std::forward<decltype(result)>(result));
            }
        } else if constexpr (std::is_member_object_pointer_v<access_ptr_type_t>) {
            auto&& result = obj.*Head;
            serializeNext(writer, std::forward<decltype(result)>(result));
        } else {
            static_assert(!sizeof(T),
                          "AccessChain: each access chain item must be a member function or member object pointer");
        }
    }

    template<BinaryReaderConcept R, typename T>
    static void deserialize(R& reader, T& obj)
    {
        if constexpr (std::is_member_function_pointer_v<access_ptr_type_t>) {
            using info_t = member_fn_info<access_ptr_type_t>;

            if constexpr (info_t::is_multi_in_setter_v) {
                static_assert(sizeof...(Tail) == 0,
                              "AccessChain: a multi-input setter (void(T...)) must be the last access chain item");
                info_t::template readAll<Head>(reader, obj);
            } else if constexpr (info_t::is_multi_out_getter_v) {
                static_assert(!sizeof(T),
                              "AccessChain: a multi-output getter cannot be used for deserialization; "
                              "provide a matching setter (void(T...)) instead");
            } else if constexpr (sizeof...(Tail) == 0) {
                static_assert(!sizeof(T),
                              "AccessChain: the last access chain item used for deserialization must be a data member "
                              "pointer or a multi-input setter - you cannot assign through a plain getter");
            } else {
                // Intermediate getter: must yield a mutable reference so we
                // can keep navigating and eventually write through it.
                auto&& result = (obj.*Head)();
                static_assert(std::is_lvalue_reference_v<decltype(result)> &&
                                !std::is_const_v<std::remove_reference_t<decltype(result)>>,
                              "AccessChain: an intermediate getter used for deserialization must return a "
                              "non-const reference");
                deserializeNext(reader, std::forward<decltype(result)>(result));
            }
        } else if constexpr (std::is_member_object_pointer_v<access_ptr_type_t>) {
            auto&& result = obj.*Head;
            deserializeNext(reader, std::forward<decltype(result)>(result));
        } else {
            static_assert(!sizeof(T),
                          "AccessChain: each access chain item must be a member function or member object pointer");
        }
    }

private:
    template<BinaryWriterConcept W, typename Value>
    static void serializeNext(W& writer, Value const& value)
    {
        using decayed_t = std::decay_t<Value>;

        if constexpr (sizeof...(Tail) == 0) {
            // Terminal access chain item: a scalar, string, or container of leaves -
            // serializeValue already recurses through nested containers.
            serializeValue(writer, value);
        } else if constexpr (std::ranges::contiguous_range<decayed_t>) {
            // More access chain items remain and this is a container: apply the rest
            // of the chain to every element (supports containers of
            // structs, not just containers of scalars).
            auto length = static_cast<serialized_container_length_type_t>(value.size());
            internal::writePrimitive(writer, length);

            for (auto const& element : value)
                AccessChainResolver<Tail...>::serialize(writer, element);
        } else {
            AccessChainResolver<Tail...>::serialize(writer, value);
        }
    }

    template<BinaryReaderConcept R, typename Value>
    static void deserializeNext(R& reader, Value& value)
    {
        using decayed_t = std::decay_t<Value>;

        if constexpr (sizeof...(Tail) == 0) {
            deserializeValue(reader, value);
        } else if constexpr (std::ranges::contiguous_range<decayed_t>) {
            auto length = serialized_container_length_type_t{};
            internal::readPrimitive(reader, length);

            value.resize(length);

            for (auto& element : value)
                AccessChainResolver<Tail...>::deserialize(reader, element);
        } else {
            AccessChainResolver<Tail...>::deserialize(reader, value);
        }
    }
};
}

template<auto... AccessChainItem>
struct AccessChain
{
    static_assert(sizeof...(AccessChainItem) > 0, "AccessChain requires at least one pointer-to-member");

    template<BinaryWriterConcept W, typename Root>
    auto serialize(W& writer, Root const& root) const -> void
    {
        internal::AccessChainResolver<AccessChainItem...>::serialize(writer, root);
    }

    template<BinaryReaderConcept R, typename Root>
    auto deserializeFrom(R& reader, Root& root) const -> void
    {
        internal::AccessChainResolver<AccessChainItem...>::deserialize(reader, root);
    }
};

template<auto... AccessChainItem>
constexpr auto makeAccessChain() noexcept
{
    return AccessChain<AccessChainItem...>{};
}

// Serializer / Deserializer
template<typename... AccessChains>
class Serializer
{
public:
    explicit Serializer(AccessChains... chains)
      : chains_{ std::move(chains)... }
    {
    }

    template<typename Root, BinaryWriterConcept Writer>
    auto serialize(Root const& root, Writer& writer) const -> void
    {
        std::apply([&](auto const&... chain) { (chain.serialize(writer, root), ...); }, chains_);
    }

private:
    std::tuple<AccessChains...> chains_;
};

template<typename... AccessChains>
class Deserializer
{
public:
    explicit Deserializer(AccessChains... chains)
      : chains_{ std::move(chains)... }
    {
    }

    template<typename Root, BinaryReaderConcept Reader>
    auto deserialize(Root& root, Reader& reader) const -> void
    {
        std::apply([&](auto const&... chain) { (chain.deserialize(reader, root), ...); }, chains_);
    }

private:
    std::tuple<AccessChains...> chains_;
};
}

#endif // CYCLONITE_SHARED_SERIALIZATION_H