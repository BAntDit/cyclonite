//
// Created by bantdit on 7/10/19.
//

#ifndef CYCLONITE_BUFFERVIEW_H
#define CYCLONITE_BUFFERVIEW_H

#include <cstddef>
#include <memory>

namespace cyclonite::core {
class BufferView
{
private:
    // std::shared_ptr<> buffer_; // what is the buffer view?

    size_t offset;
    size_t size;
    size_t stride;
};
}

// TODO:: resource manager

#endif // CYCLONITE_BUFFERVIEW_H
