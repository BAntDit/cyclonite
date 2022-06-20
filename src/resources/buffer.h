//
// Created by anton on 5/28/22.
//

#ifndef CYCLONITE_RESOURCE_BUFFER_H
#define CYCLONITE_RESOURCE_BUFFER_H

#include "buffers/bufferView.h"
#include "resource.h"
#include <array>

namespace cyclonite::resources {
class Buffer : public Resource
{
public:
    Buffer(size_t size) noexcept;

    [[nodiscard]] auto instance_tag() const -> ResourceTag const& override { return tag; }

    [[nodiscard]] auto size() const -> size_t { return dynamicDataSize(); }

    auto data() -> std::byte* { return dynamicData(); }

    template<typename DataType>
    auto view(size_t offset, size_t count, size_t stride = sizeof(DataType)) -> buffers::BufferView<DataType>;

    void load(std::istream& stream) override;

private:
    static ResourceTag tag;

public:
    static auto type_tag_const() -> ResourceTag const& { return Buffer::tag; }
    static auto type_tag() -> ResourceTag& { return Buffer::tag; }
};

template<typename DataType>
auto Buffer::view(size_t offset, size_t count, size_t stride /* = sizeof(DataType)*/) -> buffers::BufferView<DataType>
{
    return buffers::BufferView<DataType>{ data(), offset, count, stride };
}
}

#endif // CYCLONITE_RESOURCE_BUFFER_H
