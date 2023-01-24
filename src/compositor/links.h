//
// Created by anton on 4/18/21.
//

#ifndef CYCLONITE_LINKS_H
#define CYCLONITE_LINKS_H

#include "baseRenderTarget.h"
#include "uuids.h"
#include "vulkan/imageView.h"
#include "vulkan/sharedHandle.h"
#include <easy-mp/containers.h>
#include <easy-mp/enum.h>
#include <easy-mp/type_list.h>

namespace cyclonite::compositor {
using namespace easy_mp;

struct Link
{
    uint64_t nodeId;
    VkSampler sampler;
    std::array<VkImageView, value_cast(RenderTargetOutputSemantic::COUNT)> views;
    std::array<RenderTargetOutputSemantic, value_cast(RenderTargetOutputSemantic::COUNT)> semantics;
};

class Links
{
public:
    class Iterator
    {
    public:
        friend class Links;

        using iterator_category = std::random_access_iterator_tag;
        using value_type = Link;
        using difference_type = std::ptrdiff_t;
        using pointer = std::add_pointer_t<value_type>;
        using reference = std::add_lvalue_reference_t<value_type>;

        Iterator(Iterator const&) = default;

        Iterator(Iterator&&) = default;

        ~Iterator() = default;

        auto operator=(Iterator const&) -> Iterator& = default;

        auto operator=(Iterator &&) -> Iterator& = default;

        auto operator==(Iterator const&) const -> bool = default;

        auto operator<=>(Iterator const& rhs) const -> int32_t;

        auto operator*() -> reference { return *(base_ + index_); }
        [[nodiscard]] auto operator*() const -> reference { return *(base_ + index_); }

        auto operator->() -> pointer { return base_ + index_; }
        [[nodiscard]] auto operator->() const -> pointer { return base_ + index_; }

        auto operator[](difference_type index) -> reference { return *(base_ + index); }
        [[nodiscard]] auto operator[](difference_type index) const -> reference { return *(base_ + index); }

        auto operator++() -> Iterator&;
        auto operator--() -> Iterator&;

        auto operator++(int) -> Iterator;
        auto operator--(int) -> Iterator;

        auto operator+=(difference_type n) -> Iterator&;
        auto operator-=(difference_type n) -> Iterator&;

        auto operator+(difference_type n) const -> Iterator;
        auto operator-(difference_type n) const -> Iterator;

        auto operator-(Iterator const& r) const -> difference_type;
        auto operator+(Iterator const& r) const -> difference_type;

    private:
        Iterator(Links& links, difference_type index = 0) noexcept;

        Link* base_;
        difference_type index_;
        difference_type count_;
    };

    class ConstIterator
    {
    public:
        friend class Links;

        using iterator_category = std::random_access_iterator_tag;
        using value_type = std::add_const_t<Link>;
        using difference_type = std::ptrdiff_t;
        using pointer = std::add_pointer_t<std::add_const_t<value_type>>;
        using reference = std::add_lvalue_reference_t<std::add_const_t<value_type>>;

        ConstIterator(ConstIterator const&) = default;

        ConstIterator(ConstIterator&&) = default;

        ~ConstIterator() = default;

        auto operator=(ConstIterator const&) -> ConstIterator& = default;

        auto operator=(ConstIterator &&) -> ConstIterator& = default;

        auto operator==(ConstIterator const&) const -> bool = default;

        auto operator<=>(ConstIterator const& rhs) const -> int32_t;

        [[nodiscard]] auto operator*() const -> reference { return *(base_ + index_); }

        [[nodiscard]] auto operator->() const -> pointer { return base_ + index_; }

        [[nodiscard]] auto operator[](difference_type index) const -> reference { return *(base_ + index); }

        auto operator++() -> ConstIterator&;
        auto operator--() -> ConstIterator&;

        auto operator++(int) -> ConstIterator;
        auto operator--(int) -> ConstIterator;

        auto operator+=(difference_type n) -> ConstIterator&;
        auto operator-=(difference_type n) -> ConstIterator&;

        auto operator+(difference_type n) const -> ConstIterator;
        auto operator-(difference_type n) const -> ConstIterator;

        auto operator-(ConstIterator const& r) const -> difference_type;
        auto operator+(ConstIterator const& r) const -> difference_type;

    private:
        ConstIterator(Links const& links, difference_type index = 0) noexcept;

        Link const* base_;
        difference_type index_;
        difference_type count_;
    };

public:
    template<size_t linkCount>
    static auto create(vulkan::Device& device) -> Links;

    Links();

    Links(Links const&) = delete;

    Links(Links&& links) noexcept;

    ~Links();

    auto operator=(Links const&) -> Links& = delete;

    auto operator=(Links&& rhs) noexcept -> Links&;

    auto begin() -> Iterator { return Iterator{ *this, 0 }; }

    [[nodiscard]] auto begin() const -> ConstIterator { return ConstIterator{ *this, 0 }; }

    auto end() -> Iterator { return Iterator{ *this, static_cast<Iterator::difference_type>(size()) }; }

    [[nodiscard]] auto end() const -> ConstIterator
    {
        return ConstIterator{ *this, static_cast<Iterator::difference_type>(size()) };
    }

    [[nodiscard]] auto cbegin() const -> ConstIterator { return ConstIterator{ *this, 0 }; }

    [[nodiscard]] auto cend() const -> ConstIterator
    {
        return ConstIterator{ *this, static_cast<Iterator::difference_type>(size()) };
    }

    [[nodiscard]] auto size() const -> size_t;

    [[nodiscard]] auto get(size_t index) const -> Link const&;

    auto get(size_t index) -> Link&;

private:
    template<size_t maxSize>
    using image_io = to_variant_t<typename concat<type_list<std::monostate>, array_list_t<Link, maxSize>>::type>;

    static constexpr size_t maxInputCount = 64;

    image_io<maxInputCount> links_;
    VkDevice vkDevice_;
};

template<size_t linkCount>
auto Links::create(vulkan::Device& device) -> Links
{
    static_assert(linkCount <= maxInputCount);

    Links links;
    links.links_ = std::conditional_t<linkCount != 0, std::array<Link, linkCount>, std::monostate>{};
    links.vkDevice_ = device.handle();

    for (auto& [nodeId, sampler, views, semantics] : links) {
        nodeId = std::numeric_limits<size_t>::max();

        {
            auto samplerInfo = VkSamplerCreateInfo{};

            samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
            samplerInfo.magFilter = VK_FILTER_NEAREST;
            samplerInfo.minFilter = VK_FILTER_NEAREST;
            samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
            samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
            samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
            samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
            samplerInfo.mipLodBias = 0.f;
            samplerInfo.maxAnisotropy = 1.f;
            samplerInfo.minLod = 0.f;
            samplerInfo.maxLod = 1.f;
            samplerInfo.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;

            if (auto result = vkCreateSampler(device.handle(), &samplerInfo, nullptr, &sampler); result != VK_SUCCESS) {
                throw std::runtime_error("could not create link sampler");
            }
        }
        for (auto i = size_t{ 0 }; i < value_cast(RenderTargetOutputSemantic::COUNT); i++) {
            views[i] = VK_NULL_HANDLE;
            semantics[i] = RenderTargetOutputSemantic::INVALID;
        }
    }

    return links;
}
}

#endif // CYCLONITE_LINKS_H
