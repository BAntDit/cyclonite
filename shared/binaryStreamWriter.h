//
// Created by anton on 8/11/26.
//

#ifndef CYCLONITE_SHARED_BINARY_STREAM_WRITER_H
#define CYCLONITE_SHARED_BINARY_STREAM_WRITER_H

#include "serializationCommon.h"
#include <ostream>
#include <stdexcept>

namespace cyclonite::shared {
class BinaryStreamWriter
{
public:
    BinaryStreamWriter(std::ostream& stream, Endian endian = Endian::native)
      : stream_{ stream }
      , endian_{ endian }
    {
    }

    void writeBytes(void const* data, size_t size)
    {
        stream_.write(reinterpret_cast<char const*>(data), static_cast<std::streamsize>(size));
        if (!stream_)
            throw std::runtime_error("BinaryStreamWriter: write failed");
    }

    [[nodiscard]] auto endianness() const -> Endian { return endian_; }

private:
    std::ostream& stream_;
    Endian endian_;
};
}

#endif // CYCLONITE_SHARED_BINARY_STREAM_WRITER_H