//
// Created by bantdit on 7/10/19.
//

#ifndef CYCLONITE_BUFFERVIEW_H
#define CYCLONITE_BUFFERVIEW_H

#include <cstddef>
#include <cstdint>
#include <memory>

namespace cyclonite::core {
class BufferView
{
public:
private:
    uint32_t stagingBufferId_;
    uint32_t deviceBufferId_;

    size_t stagingOffset_;
    size_t deviceOffset_;
    size_t size_;
    size_t stride_;
};
}

// TODO:: resource manager

#endif // CYCLONITE_BUFFERVIEW_H
