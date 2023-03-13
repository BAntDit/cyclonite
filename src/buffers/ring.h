//
// Created by anton on 3/13/23.
//

#ifndef CYCLONITE_RING_H
#define CYCLONITE_RING_H

#include <cstddef>

namespace cyclonite::buffers
{
template<typename MemoryPage>
class Ring
{
public:

private:
    size_t size_;
    size_t align_;
    size_t head_;
    size_t tail_;
};
}

#endif // CYCLONITE_RING_H
