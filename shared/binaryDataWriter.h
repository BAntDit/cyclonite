//
// Created by anton on 1/7/26.
//

#ifndef CYCLONITE_BINARY_DATA_WRITER_H
#define CYCLONITE_BINARY_DATA_WRITER_H

#include <boost/iostreams/stream.hpp>
#include <filesystem>
#include <fstream>
#include <metrix/containers.h>
#include <tuple>

namespace cyclonite::shared {
class BinaryStreamWriter
{
public:
    BinaryStreamWriter(std::filesystem::path const& path)
      : output_{}
    {
        output_.exceptions(std::ios::failbit);
        output_.open(path.string(), std::ios::out | std::ios::binary);
        output_.exceptions(std::ios::badbit);
    }

    BinaryStreamWriter(std::fstream&& stream)
      : output_(std::move(stream))
    {
    }

    ~BinaryStreamWriter()
    {
        if (output_.is_open()) {
            output_.close();
        }
    }

    template<typename T>
        requires metrix::is_iterable_v<std::decay_t<T>> && metrix::is_contiguous_v<std::decay_t<T>>
    void operator<<(T const& t);

    template<typename... Args>
    void operator<<(std::tuple<Args...> const& tuple);

    template<typename T>
        requires std::is_integral_v<T> || std::is_floating_point_v<T>
    void operator<<(T t);

    void operator<<(std::string const& string);

private:
    std::fstream output_;
};

template<typename T>
    requires metrix::is_iterable_v<std::decay_t<T>> && metrix::is_contiguous_v<std::decay_t<T>>
inline void BinaryStreamWriter::operator<<(T const& t)
{
    auto size = std::size(t);
    output_.write(static_cast<char const*>(&size), sizeof(size));
    for (auto const& e : t) {
        output_.write(static_cast<char const*>(&e), sizeof(e));
    }
}

template<typename... Args>
inline void BinaryStreamWriter::operator<<(std::tuple<Args...> const& tuple)
{
    auto s = [this]<size_t... idx>(std::tuple<Args...> const& t, std::index_sequence<idx...>) -> void {
        (((*this) << std::get<idx>(t)), ...);
    };
    s(tuple, std::make_index_sequence<sizeof...(Args)>{});
}

template<typename T>
    requires std::is_integral_v<T> || std::is_floating_point_v<T>
inline void BinaryStreamWriter::operator<<(T t)
{
    auto* ptr = reinterpret_cast<char const*>(&t);
    output_.write(ptr, sizeof(t));
}

inline void BinaryStreamWriter::operator<<(std::string const& string)
{
    auto size = string.size();
    auto* ptr = reinterpret_cast<char const*>(&size);
    output_.write(ptr, sizeof(size));
    output_.write(string.data(), size);
}
}

#endif // CYCLONITE_BINARY_DATA_WRITER_H