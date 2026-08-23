//
// Created by anton on 8/11/26.
//

#ifndef CYCLONITE_SHARED_BINARY_STREAM_READER_H
#define CYCLONITE_SHARED_BINARY_STREAM_READER_H

#include "serializationCommon.h"
#include <cassert>
#include <istream>
#include <stdexcept>

namespace cyclonite::shared {
class BinaryStreamReader
{
public:
    BinaryStreamReader(std::istream& stream, Endian endian = Endian::Native)
      : stream_{ &stream }
      , endian_{ endian }
    {
        assert(stream_ != nullptr);
    }

    void setStreamOffset(size_t offset)
    {
        assert(stream_ != nullptr);
        stream_->seekg(static_cast<int32_t>(offset), std::ios::beg);
    }

    auto readBytes(void* data, size_t size) -> void
    {
        assert(stream_ != nullptr);
        if (!stream_->read(reinterpret_cast<char*>(data), static_cast<std::streamsize>(size))) {
            throw std::runtime_error("BinaryStreamReader: read failed");
        }
    }

    [[nodiscard]] auto endianness() const -> Endian { return endian_; }

private:
    std::istream* stream_;
    Endian endian_;
};
}

#endif // CYCLONITE_SHARED_BINARY_STREAM_READER_H