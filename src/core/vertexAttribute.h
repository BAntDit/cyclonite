//
// Created by bantdit on 5/27/19.
//

#ifndef CYCLONITE_VERTEXATTRIBUTE_H
#define CYCLONITE_VERTEXATTRIBUTE_H

#include <cstddef>

#include "attributeSemantic.h"
#include "../../../../../../usr/include/c++/7/cstdint"

namespace cyclonite::core {
template<typename Type, AttributeSemantic S>
class VertexAttribute {
public:

private:
    bool normalized_;

    size_t offset_;
    size_t stride_;

    uint32_t count_;

    // get count

    // count components

    // get value by index
};
}

#endif // CYCLONITE_VERTEXATTRIBUTE_H
