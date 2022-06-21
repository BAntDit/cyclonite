//
// Created by anton on 6/22/22.
//

#ifndef CYCLONITE_ANIMTAIONS_ANIMATION_H
#define CYCLONITE_ANIMATIONS_ANIMATION_H

#include "resources/resource.h"
#include "sampler.h"

namespace cyclonite::animations {
class Animation : public resources::Resource
{
public:
    class Iterator
    {
    public:
        using iterator_category = std::random_access_iterator_tag;
        using value_type = Sampler;
        using difference_type = std::ptrdiff_t;
        using pointer = value_type*;
        using reference = value_type&;

        Iterator(Sampler* baseSampler, uint32_t samplerCount, difference_type index) noexcept;

        explicit operator bool() const { return index_ < count_; }

        auto operator==(Iterator const& rhs) const -> bool
        {
            return baseSampler_ == rhs.baseSampler_ && index_ == rhs.index_;
        }

        auto operator!=(Iterator const& rhs) const -> bool
        {
            return baseSampler_ != rhs.baseSampler_ || index_ != rhs.index_;
        }

        auto operator+=(difference_type diff) -> Iterator&;

        auto operator-=(difference_type diff) -> Iterator&;

        auto operator++() -> Iterator&;

        auto operator--() -> Iterator&;

        auto operator++(int) -> Iterator;

        auto operator--(int) -> Iterator;

        auto operator+(difference_type diff) -> Iterator;

        auto operator-(difference_type diff) -> Iterator;

        auto operator-(Iterator const& rhs) const -> difference_type;

        auto operator*() -> Sampler&;

        auto operator*() const -> Sampler const&;

        auto operator->() const -> Sampler*;

        auto ptr() const -> Sampler*;

        auto operator[](int index) const -> reference;

    private:
        Sampler* baseSampler_;
        difference_type index_;
        uint32_t count_;
    };

public:
    explicit Animation(uint32_t samplerCount) noexcept;

    [[nodiscard]] auto instance_tag() const -> ResourceTag const& override { return tag; }

    [[nodiscard]] auto begin() const -> Iterator { return Iterator{ baseSampler_, samplerCount_, 0l }; }

    [[nodiscard]] auto end() const -> Iterator
    {
        return Iterator{ baseSampler_, samplerCount_, static_cast<typename Iterator::difference_type>(samplerCount_) };
    }

protected:
    void handleDynamicDataAllocation() override;

private:
    Sampler* baseSampler_;
    uint32_t samplerCount_;

private:
    static ResourceTag tag;

public:
    static auto type_tag_const() -> ResourceTag const& { return Animation::tag; }
    static auto type_tag() -> ResourceTag& { return Animation::tag; }
};
}

#endif // CYCLONITE_ANIMATIONS_ANIMATION_H
