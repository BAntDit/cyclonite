//
// Created by anton on 12/19/20.
//

#include "node.h"

namespace cyclonite::compositor {
void Node::Links::set(vulkan::ImagePtr const& image, size_t index)
{
    std::visit(
      [&](auto&& links) -> void {
          if constexpr (!std::is_same_v<std::decay_t<decltype(links)>, std::monostate>) {
              assert(index < links.size());

              links[index] = image;
          }
      },
      links_);
}

auto Node::Links::get(size_t index) const -> vulkan::ImagePtr const&
{
    return std::visit(
      [&](auto&& links) -> vulkan::ImagePtr const& {
          if constexpr (!std::is_same_v<std::decay_t<decltype(links)>, std::monostate>) {
              assert(index < links.size());

              return links[index];
          }

          std::terminate();
      },
      links_);
}
}