//
// Created by bantdit on 5/27/19.
//

#ifndef CYCLONITE_VERTEXATTRIBUTE_H
#define CYCLONITE_VERTEXATTRIBUTE_H

#include <cstddef>
#include <cstdint>
#include <easy-mp/enum.h>
#include <vector>

#include "attributeSemantic.h"

namespace cyclonite::core {
template<typename Type, AttributeSemantic S>
class VertexAttribute
{
public:
    using type_t = Type;

    using semantic_t = std::integral_constant<std::underlying_type_t<AttributeSemantic>, easy_mp::value_cast(S)>;

    static constexpr AttributeSemantic semantic_v = S;

private:
    bool normalized_;

    size_t offset_;
    size_t stride_;

    uint32_t count_;

    // get count

    // count components

    // get value by index

    // TODO:: buffer view
};
}

#endif // CYCLONITE_VERTEXATTRIBUTE_H
