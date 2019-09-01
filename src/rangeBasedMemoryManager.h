//
// Created by bantdit on 8/31/19.
//

#ifndef CYCLONITE_RANGEBASEDMEMORYMANAGER_H
#define CYCLONITE_RANGEBASEDMEMORYMANAGER_H

#include <cstddef>
#include <deque>

namespace cyclonite {
class RangeBasedMemoryManager
{
public:
    RangeBasedMemoryManager();

    auto allocMemory(size_t size) -> std::pair<std::byte*, size_t>;

    void releaseMemory(std::pair<size_t, size_t> range);

private:
    std::deque<std::pair<size_t, size_t>> ranges_;
};
}

#endif // CYCLONITE_RANGEBASEDMEMORYMANAGER_H
