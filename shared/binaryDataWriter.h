//
// Created by anton on 1/7/26.
//

#ifndef CYCLONITE_BINARY_DATA_WRITER_H
#define CYCLONITE_BINARY_DATA_WRITER_H

#include <filesystem>
#include <fstream>
#include <tuple>
#include <boost/iostreams/device/array.hpp>
#include <boost/iostreams/stream.hpp>

namespace cyclonite::shared
{
namespace internal
{
template<typename T, typename = void>
struct is_iterable : std::false_type
{};

template<typename T>
struct is_iterable<T, std::void_t<decltype(std::declval<T>().begin()), decltype(std::declval<T>().end())>>
  : std::true_type
{};

template<typename T>
inline constexpr bool is_iterable_v = is_iterable<T>::value;
}

class SerializationDataWriter
{
public:
	SerializationDataWriter(std::filesystem::path const& path) : output_{} {
		output_.exceptions(std::ios::failbit);
		output_.open(path.string(), std::ios::out | std::ios::binary);
		output_.exceptions(std::ios::badbit);
	}

	SerializationDataWriter(std::fstream&& stream) : output_(std::move(stream)) {}

	~SerializationDataWriter() {
		if (output_.is_open()) {
			output_.close();
		}
	}

	// template<typename... Data, template <typename...> typename Tuple>
	// void operator()(Tuple<Data...> const& data);

	template<typename T, typename std::enable_if_t<std::is_integral_v<std::decay_t<T>> || std::is_floating_point_v<std::decay_t<T>>, int> = 0>
	void operator()(T t);

	// template<typename T, size_t N, typename std::enable_if_t<std::is_integral_v<T> || std::is_floating_point_v<T>, int> = 0>
	// void operator()(const T(&a)[N]);

	// template<typename C, typename std::enable_if_t<internal::is_iterable_v<C>, int> = 0>
	// void operator()(C&& container);

private:
	std::fstream output_;
};

/*template<typename... Data, template <typename...> typename Tuple>
void SerializationDataWriter::operator()(Tuple<Data...> const& data)
{
	auto s = [this]<size_t... idx>(auto&& t, std::index_sequence<idx...>) -> void {
		((*this)(std::get<idx>(t)), ...);
	};

	s(data, std::make_index_sequence<sizeof...(Data)>{});
}*/

template<typename T, typename std::enable_if_t<std::is_integral_v<std::decay_t<T>> || std::is_floating_point_v<std::decay_t<T>>, int>>
void SerializationDataWriter::operator()(T t) {
	auto* ptr = reinterpret_cast<char const*>(&t);
	output_.write(ptr, sizeof(t));
}

/*template<typename T, size_t N, typename std::enable_if_t<std::is_integral_v<T> || std::is_floating_point_v<T>, int>>
void SerializationDataWriter::operator()(const T(&a)[N]) {
	for (auto i = size_t{0}; i < N; i++) {
		(*this)(a[i]);
	}
}

template<typename C, typename std::enable_if_t<internal::is_iterable_v<C>, int>>
void SerializationDataWriter::operator()(C&& container)
{
	for (auto&& e : container) {
		(*this)(std::forward<decltype(e)>(e));
	}
}*/
}

#endif //CYCLONITE_BINARY_DATA_WRITER_H