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
    Handle()
      : Handle([](T, VkAllocationCallbacks*) -> void {})
    {}

    explicit Handle(void deleter(T, VkAllocationCallbacks*))
      : handle_{ VK_NULL_HANDLE }
      , deleter_{ deleter }
    {}

    Handle(VkInstance vkInstance, void deleter(VkInstance, T, VkAllocationCallbacks*))
      : handle_{ VK_NULL_HANDLE }
      , deleter_{}
    {
        deleter_ = [deleter, vkInstance](T handle, VkAllocationCallbacks* vkAllocationCallbacks) -> void {
            deleter(vkInstance, handle, vkAllocationCallbacks);
        };
    }

    Handle(VkDevice vkDevice, void deleter(VkDevice, T, VkAllocationCallbacks*))
      : handle_{ VK_NULL_HANDLE }
      , deleter_{}
    {
        deleter_ = [deleter, vkDevice](T handle, VkAllocationCallbacks* vkAllocationCallbacks) -> void {
            deleter(vkDevice, handle, vkAllocationCallbacks);
        };
    }

    Handle(Handle&& handle) = default;

    Handle(Handle const& handle) = delete;

    ~Handle() { reset(); }

    auto operator=(Handle const& rhs) -> Handle& = delete;

    auto operator=(Handle&& rhs) -> Handle&
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
    std::function<void(T)> deleter_;
};
}

#endif // CYCLONITE_VULKAN_HANDLE_H