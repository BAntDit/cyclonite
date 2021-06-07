//
// Created by anton on 4/18/21.
//

#ifndef CYCLONITE_LINKS_H
#define CYCLONITE_LINKS_H

#include "vulkan/imageView.h"
#include <easy-mp/containers.h>
#include <easy-mp/type_list.h>

namespace cyclonite::compositor {
using namespace easy_mp;

class Links
{
public:
    template<size_t linkCount>
    static auto create() -> Links;

    Links() = default;

    void set(vulkan::ImageView const& imageView, size_t index);

    [[nodiscard]] auto get(size_t index) const -> vulkan::ImageView const&;

    [[nodiscard]] auto size() const -> size_t;

private:
    // TODO:: store shared pointers instead

    template<size_t maxSize>
    using image_io =
      to_variant_t<typename concat<type_list<std::monostate>, array_list_t<vulkan::ImageView const*, maxSize>>::type>;

    static constexpr size_t maxInputCount = 64;

    image_io<maxInputCount> links_;
};

template<size_t linkCount>
auto Links::create() -> Links
{
    static_assert(linkCount <= maxInputCount);

    Links links;
    links.links_ =
      std::conditional_t<linkCount != 0, std::array<vulkan::ImageView const*, linkCount>, std::monostate>{};

    return links;
}
}

#endif // CYCLONITE_LINKS_H
