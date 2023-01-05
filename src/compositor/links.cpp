//
// Created by anton on 12/19/20.
//

#include "links.h"

namespace cyclonite::compositor {
Links::Iterator::Iterator(Links& links, difference_type index) noexcept
  : base_{ nullptr }
  , index_{ index }
  , count_{ 0 }
{
    std::visit(
      [this](auto&& l) -> void {
          if constexpr (!std::is_same_v<std::decay_t<decltype(l)>, std::monostate>) {
              base_ = l.data();
              count_ = l.size();

              assert(index_ <= count_);
          }
      },
      links.links_);
}

auto Links::Iterator::operator<=>(Links::Iterator const& rhs) const -> int32_t
{
    assert(base_ == rhs.base_); // comparing iterator from different containers is UB

    if (index_ < rhs.index_)
        return -1;
    if (index_ > rhs.index_)
        return 1;

    return 0;
}

auto Links::Iterator::operator++() -> Iterator&
{
    index_++;
    return *this;
}

auto Links::Iterator::operator--() -> Iterator&
{
    index_--;
    return *this;
}

auto Links::Iterator::operator++(int) -> Iterator
{
    Iterator r{ *this };
    index_++;
    return r;
}

auto Links::Iterator::operator--(int) -> Iterator
{
    Iterator r{ *this };
    index_--;
    return r;
}

auto Links::Iterator::operator+=(difference_type n) -> Iterator&
{
    index_ += n;
    return *this;
}

auto Links::Iterator::operator-=(difference_type n) -> Iterator&
{
    index_ -= n;
    return *this;
}

auto Links::Iterator::operator+(difference_type n) const -> Iterator
{
    Iterator r{ *this };
    return r += n;
}

auto Links::Iterator::operator-(difference_type n) const -> Iterator
{
    Iterator r{ *this };
    return r -= n;
}

auto Links::Iterator::operator-(Iterator const& r) const -> difference_type
{
    return index_ - r.index_;
}

auto Links::Iterator::operator+(Iterator const& r) const -> difference_type
{
    return index_ + r.index_;
}

auto Links::size() const -> size_t
{
    return std::visit(
      [](auto&& links) -> size_t {
          if constexpr (!std::is_same_v<std::decay_t<decltype(links)>, std::monostate>) {
              return links.size();
          }

          return 0;
      },
      links_);
}

auto Links::ConstIterator::operator<=>(ConstIterator const& rhs) const -> int32_t
{
    assert(base_ == rhs.base_); // comparing iterator from different containers is UB

    if (index_ < rhs.index_)
        return -1;
    if (index_ > rhs.index_)
        return 1;
    if (index_ < rhs.index_)
        return -1;
    if (index_ > rhs.index_)
        return 1;

    return 0;
}

auto Links::ConstIterator::operator++() -> ConstIterator&
{
    index_++;
    return *this;
}

auto Links::ConstIterator::operator--() -> ConstIterator&
{
    index_--;
    return *this;
}

auto Links::ConstIterator::operator++(int) -> ConstIterator
{
    ConstIterator r{ *this };
    index_++;
    return r;
}

auto Links::ConstIterator::operator--(int) -> ConstIterator
{
    ConstIterator r{ *this };
    index_--;
    return r;
}

auto Links::ConstIterator::operator+=(difference_type n) -> ConstIterator&
{
    index_ += n;
    return *this;
}

auto Links::ConstIterator::operator-=(difference_type n) -> ConstIterator&
{
    index_ -= n;
    return *this;
}

auto Links::ConstIterator::operator+(difference_type n) const -> ConstIterator
{
    ConstIterator r{ *this };
    return r += n;
}

auto Links::ConstIterator::operator-(difference_type n) const -> ConstIterator
{
    ConstIterator r{ *this };
    return r -= n;
}

auto Links::ConstIterator::operator-(ConstIterator const& r) const -> difference_type
{
    return index_ - r.index_;
}

auto Links::ConstIterator::operator+(ConstIterator const& r) const -> difference_type
{
    return index_ + r.index_;
}

Links::ConstIterator::ConstIterator(Links const& links, difference_type index) noexcept
  : base_{ nullptr }
  , index_{ index }
  , count_{ 0 }
{
    std::visit(
      [this](auto const& l) -> void {
          if constexpr (!std::is_same_v<std::decay_t<decltype(l)>, std::monostate>) {
              base_ = l.data();
              count_ = l.size();

              assert(index_ <= count_);
          }
      },
      links.links_);
}

[[nodiscard]] auto Links::get(size_t index) const -> Link const&
{
    return std::visit(
      [index](auto const& l) -> Link const& {
          if constexpr (!std::is_same_v<std::decay_t<decltype(l)>, std::monostate>) {
              return l[index];
          }

          std::terminate();
      },
      links_);
}

auto Links::get(size_t index) -> Link&
{
    return const_cast<Link&>(std::as_const(*this).get(index));
}

Links::Links() :
  links_{}
  , vkDevice_{ VK_NULL_HANDLE }
{}

Links::Links(Links&& links) noexcept :
  links_(links.links_)
  , vkDevice_{ links.vkDevice_ }
{
    links.vkDevice_ = VK_NULL_HANDLE;
}

Links::~Links()
{
    if (vkDevice_ != VK_NULL_HANDLE) {
        for (auto& link : (*this)) {
            vkDestroySampler(vkDevice_, link.sampler, nullptr);
        }
    }
}

auto Links::operator=(Links&& rhs) noexcept -> Links&
{
    links_ = rhs.links_;
    vkDevice_ = rhs.vkDevice_;
    rhs.vkDevice_ = VK_NULL_HANDLE;

    return *this;
}
}