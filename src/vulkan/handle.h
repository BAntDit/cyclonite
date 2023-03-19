//
// Created by bantdit on 2/24/19.
//

#ifndef CYCLONITE_VULKAN_HANDLE_H
#define CYCLONITE_VULKAN_HANDLE_H

#include <cassert>
#include <functional>
#include <vulkan/vulkan.h>

namespace cyclonite::vulkan {
template<typename T>
class Handle
{
public:
    Handle() noexcept
      : Handle([](T, VkAllocationCallbacks const*) -> void {})
    {
    }

    explicit Handle(void (*deleter)(T, VkAllocationCallbacks const*))
      : handle_{ VK_NULL_HANDLE }
      , deleter_{ deleter }
    {
    }

    Handle(VkInstance vkInstance, void (*deleter)(VkInstance, T, VkAllocationCallbacks const*))
      : handle_{ VK_NULL_HANDLE }
      , deleter_{}
    {
        deleter_ = [deleter, vkInstance](T handle, VkAllocationCallbacks const* vkAllocationCallbacks) -> void {
            deleter(vkInstance, handle, vkAllocationCallbacks);
        };
    }

    Handle(VkDevice vkDevice, void (*deleter)(VkDevice, T, VkAllocationCallbacks const*))
      : handle_{ VK_NULL_HANDLE }
      , deleter_{}
    {
        deleter_ = [deleter, vkDevice](T handle, VkAllocationCallbacks const* vkAllocationCallbacks) -> void {
            deleter(vkDevice, handle, vkAllocationCallbacks);
        };
    }

    Handle(Handle&& handle) noexcept
      : handle_{ handle.handle_ }
      , deleter_{ std::move(handle.deleter_) }
    {
        handle.handle_ = VK_NULL_HANDLE;
    }

    Handle(Handle const& handle) = delete;

    ~Handle() { reset(); }

    auto operator=(Handle const& rhs) -> Handle& = delete;

    auto operator=(Handle&& rhs) noexcept -> Handle&
    {
        reset();

        handle_ = rhs.handle_;
        deleter_ = std::move(rhs.deleter_);

        rhs.handle_ = VK_NULL_HANDLE;

        return *this;
    }

    explicit operator T() const { return handle_; }

    explicit operator bool() const { return handle_ != VK_NULL_HANDLE; }

    auto operator&() const -> T const* { return &handle_; }

    auto operator&() -> T*
    {
        assert(handle_ == VK_NULL_HANDLE); // to avoid accidentally replace initialized handler
        return _replace();
    }

    auto operator==(Handle const& rhs) const -> bool { return handle_ == rhs.handle_; }

    auto operator!=(Handle const& rhs) const -> bool { return handle_ != rhs.handle_; }

    void reset()
    {
        if (handle_ != VK_NULL_HANDLE) {
            deleter_(handle_, nullptr);
            handle_ = VK_NULL_HANDLE;
        }
    }

    auto _replace() -> T*
    {
        reset();
        return &handle_;
    }

private:
    T handle_;
    std::function<void(T, VkAllocationCallbacks const*)> deleter_;
};
}

#endif // CYCLONITE_VULKAN_HANDLE_H
