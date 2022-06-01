//
// Created by anton on 5/28/22.
//

#ifndef CYCLONITE_RESOURCE_BUFFER_H
#define CYCLONITE_RESOURCE_BUFFER_H

#include "bufferView.h"
#include "resource.h"
#include <array>

namespace cyclonite::resources {
class Buffer : public Resource
{
public:
    explicit Buffer(size_t size) noexcept;

    [[nodiscard]] auto instance_tag() const -> ResourceTag const& override { return tag; }

    [[nodiscard]] auto size() const -> size_t { return dynamicDataSize(); }

    auto data() -> std::byte* { return dynamicData(); }

    template<typename DataType>
    auto view(size_t offset, size_t count, size_t stride = sizeof(DataType)) -> BufferView<DataType>;

    void load(std::istream& stream) override;

public:
    static ResourceTag tag;
    static auto type_tag_const() -> ResourceTag const& { return tag; }
    static auto type_tag() -> ResourceTag& { return tag; }
};

template<typename DataType>
auto Buffer::view(size_t offset, size_t count, size_t stride /* = sizeof(DataType)*/) -> BufferView<DataType>
{
    return BufferView<DataType>{ data(), offset, count, stride };
}
}

#endif // CYCLONITE_RESOURCE_BUFFER_H
