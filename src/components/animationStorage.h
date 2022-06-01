//
// Created by anton on 5/21/22.
//

#ifndef CYCLONITE_ANIMATIONSTORAGE_H
#define CYCLONITE_ANIMATIONSTORAGE_H

#include "animation.h"
#include <enttx/enttx.h>
#include <set>

namespace cyclonite::components {
class AnimationStorage : public enttx::BaseComponentStorage<AnimationStorage, Animation>
{
public:
    AnimationStorage();

    [[nodiscard]] auto get(uint32_t index) const -> Animation const&;

    auto get(uint32_t index) -> Animation&;

    auto create(uint32_t index, uint16_t channelCount) -> Animation&;

    void destroy(uint32_t index);

    [[nodiscard]] auto capacity() const -> size_t { return animations_.capacity(); }

    [[nodiscard]] auto size() const -> size_t { return animations_.size(); }

    auto begin() const { return animations_.cbegin(); }

    auto end() const { return animations_.cend(); }

    auto begin() { return animations_.begin(); }

    auto end() { return animations_.end(); }

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
    std::vector<Animation> animations_;
    std::set<std::pair<size_t, size_t>, free_range_comparator> freeRanges_;
    std::vector<uint32_t> freeIndices_; // mesh indices
    std::vector<uint32_t> indices_;
};
}

#endif // CYCLONITE_ANIMATIONSTORAGE_H
