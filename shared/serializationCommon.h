//
// Created by anton on 8/11/26.
//

#ifndef CYCLONITE_SHARED_SERIALIZATION_COMMON_H
#define CYCLONITE_SHARED_SERIALIZATION_COMMON_H

#include <array>
#include <bit>
#include <cstddef>
#include <cstdint>

namespace cyclonite::shared {
enum class Endian : uint8_t
{
    little,
    big,
    native = (std::endian::native == std::endian::little) ? little : big
};

namespace internal {
template<typename T>
    requires std::is_trivially_copyable_v<T> && (sizeof(T) <= 8)
constexpr auto byteSwap(T value) noexcept -> T
{
    if constexpr (sizeof(T) == 1) {
        return value;
    } else {
        auto bytes = std::bit_cast<std::array<std::byte, sizeof(T)>>(value);
        auto swapped = std::array<std::byte, sizeof(T)>{};

        for (size_t i = 0; i < sizeof(T); i++)
            swapped[i] = bytes[(sizeof(T) - 1) - i];

        return std::bit_cast<T>(swapped);
    }
}
}

template<typename T>
constexpr auto toEndian(T value, Endian target) noexcept -> T
{
    if (target == Endian::native)
        return value;
    return internal::byteSwap(value);
}

template<typename T>
constexpr auto fromEndian(T value, Endian source) noexcept -> T
{
    return toEndian(value, source);
}

template<typename W>
concept BinaryWriterConcept = requires(W w, void const* data, size_t size) {
    { w.writeBytes(data, size) } -> std::same_as<void>;
    { w.endianness() } -> std::same_as<Endian>;
};

template<typename R>
concept BinaryReaderConcept = requires(R r, void* data, size_t size) {
    { r.readBytes(data, size) } -> std::same_as<void>;
    { r.endianness() } -> std::same_as<Endian>;
};
}

#endif // CYCLONITE_SHARED_SERIALIZATION_COMMON_H