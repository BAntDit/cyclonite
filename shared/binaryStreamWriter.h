//
// Created by anton on 8/11/26.
//

#ifndef CYCLONITE_SHARED_BINARY_STREAM_WRITER_H
#define CYCLONITE_SHARED_BINARY_STREAM_WRITER_H

#include "serializationCommon.h"
#include <filesystem>
#include <memory>
#include <ostream>
#include <fstream>
#include <stdexcept>
#include <cassert>

namespace cyclonite::shared {
class BinaryStreamWriter
{
public:
    BinaryStreamWriter(std::filesystem::path const& path, Endian endian = Endian::Native)
      : stream_{ std::make_unique<std::ofstream>(path.string(), std::ios::out | std::ios::binary) }
      , endian_{ endian }
    {
    }

    BinaryStreamWriter(std::ofstream&& stream, Endian endian = Endian::Native)
      : stream_{ std::make_unique<std::ofstream>(std::move(stream)) }
      , endian_{ endian }
    {
    }

    BinaryStreamWriter(BinaryStreamWriter&&) = default;

    BinaryStreamWriter(BinaryStreamWriter const&) = delete;

    auto operator=(BinaryStreamWriter&&) -> BinaryStreamWriter& = default;

    auto operator=(BinaryStreamWriter const&) -> BinaryStreamWriter& = delete;

    void writeBytes(void const* data, size_t size)
    {
        assert(stream_ != nullptr);
        stream_->write(reinterpret_cast<char const*>(data), static_cast<std::streamsize>(size));
        if (!stream_)
            throw std::runtime_error("BinaryStreamWriter: write failed");
    }

    [[nodiscard]] auto endianness() const -> Endian { return endian_; }

private:
    std::unique_ptr<std::ostream> stream_;
    Endian endian_;
};
}

#endif // CYCLONITE_SHARED_BINARY_STREAM_WRITER_H