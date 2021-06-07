//
// Created by anton on 12/19/20.
//

#include "links.h"

namespace cyclonite::compositor {
void Links::set(vulkan::ImageView const& imageView, size_t index)
{
    std::visit(
      [&](auto&& links) -> void {
          if constexpr (!std::is_same_v<std::decay_t<decltype(links)>, std::monostate>) {
              assert(index < links.size());

              links[index] = &imageView;
          }
      },
      links_);
}

auto Links::get(size_t index) const -> vulkan::ImageView const&
{
    return std::visit(
      [&](auto&& links) -> vulkan::ImageView const& {
          if constexpr (!std::is_same_v<std::decay_t<decltype(links)>, std::monostate>) {
              assert(index < links.size());

              return *links[index];
          }

          std::terminate();
      },
      links_);
}

auto Links::size() const -> size_t
{
    return std::visit(
      [](auto&& links) -> size_t {
          if constexpr (!std::is_same_v<std::decay_t<decltype(links)>, std::monostate>) {
              return links.size();
          }

          std::terminate();
      },
      links_);
}
}