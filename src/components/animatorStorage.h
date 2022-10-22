//
// Created by anton on 5/21/22.
//

#ifndef CYCLONITE_ANIMATORSTORAGE_H
#define CYCLONITE_ANIMATORSTORAGE_H

#include "animator.h"
#include <enttx/enttx.h>
#include <set>

namespace cyclonite::components {
class AnimatorStorage : public enttx::BaseComponentStorage<AnimatorStorage, Animator>
{
public:
    AnimatorStorage();

    [[nodiscard]] auto get(uint32_t index) const -> Animator const&;

    auto get(uint32_t index) -> Animator&;

    auto create(uint32_t index, uint16_t channelCount) -> Animator&;

    void destroy(uint32_t index);

    [[nodiscard]] auto capacity() const -> size_t { return animators_.capacity(); }

    [[nodiscard]] auto size() const -> size_t { return animators_.size(); }

    auto begin() const { return animators_.cbegin(); }

    auto end() const { return animators_.cend(); }

    auto begin() { return animators_.begin(); }

    auto end() { return animators_.end(); }

private:
    auto allocChannelRange(uint16_t channelCount) -> std::pair<size_t, size_t>;

private:
    // comparator is not a lambda to avoid subobject-linkage warning
    struct free_range_comparator
    {
        auto operator()(std::pair<size_t, size_t> const& a, std::pair<size_t, size_t> const& b) const -> bool
        {
            return a.second < b.second;
        }
    };

    std::vector<AnimationChannel> store_;
    std::vector<Animator> animators_;
    std::set<std::pair<size_t, size_t>, free_range_comparator> freeRanges_;
    std::vector<uint32_t> freeIndices_; // mesh indices
    std::vector<uint32_t> indices_;
};
}

#endif // CYCLONITE_ANIMATORSTORAGE_H
