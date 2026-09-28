
#ifndef CYCLONITE_CORE_ARENA_H
#define CYCLONITE_CORE_ARENA_H

#include <utility>
#include <map>

namespace cyclonite::core 
{
class Arena
{
public:
    struct AllocInfo
    {
        void* ptr;
        size_t rangeOffset;
        size_t rangeSize;
        size_t rangeAlign;
    };

    explicit Arena(size_t capacity);

    auto alloc(void* basePtr, size_t requiredSize, size_t offsetAlignment = 1, size_t sizeAlignment = 1) -> AllocInfo;

    auto alloc(size_t requiredSize, size_t offsetAlignment = 1, size_t sizeAlignment = 1) -> AllocInfo;

    void free(size_t offset, size_t size);

    void freeAll();

    [[nodiscard]] auto capacity() const -> size_t { return capacity_; }

    [[nodiscard]] auto getAllocatedSize() const -> size_t;

private:
    struct AllocInfoInternal
    {
        void* ptr;
        size_t rangeOffset;
        size_t rangeSize;
        size_t rangeAlign;
        bool makeNewAttempt;
    };

    auto AllocInternal(std::byte* const basePtr,
                       size_t requiredSize,
                       size_t offsetAlignment = 1,
                       size_t sizeAlignment = 1) -> AllocInfoInternal;

    using free_range_map_t = std::multimap<size_t, size_t>; // size, offset
    // https://en.cppreference.com/w/cpp/container/multimap/insert
    // (No iterators or references are invalidated)
    // https://en.cppreference.com/w/cpp/container/multimap/erase
    // (References and iterators to the erased elements are invalidated. Other references and iterators are not
    // affected.)
    using free_range_it_t = typename std::multimap<size_t, size_t>::iterator;
    using free_range_offset_map_t = std::multimap<size_t, free_range_it_t>; // offset, it to range

    free_range_map_t freeRanges_;
    free_range_offset_map_t freeRangeOffsets_;
    size_t capacity_;
};
}

#endif // CYCLONITE_CORE_ARENA_H
