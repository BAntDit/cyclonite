//
// Created by anton on 1/15/22.
//

#ifndef CYCLONITE_SHARED_HANDLE_H
#define CYCLONITE_SHARED_HANDLE_H

#include <atomic>
#include <cassert>
#include <functional>
#include <vulkan/vulkan.h>

namespace cyclonite::vulkan {
template<typename T>
class WeakHandle;

template<typename T>
class SharedHandle
{
    friend class WeakHandle<T>;

public:
    SharedHandle() noexcept;

    explicit SharedHandle(void (*deleter)(T, VkAllocationCallbacks const*)) noexcept;

    SharedHandle(VkInstance vkInstance, void (*deleter)(VkInstance, T, VkAllocationCallbacks const*)) noexcept;

    SharedHandle(VkDevice vkDevice, void (*deleter)(VkDevice, T, VkAllocationCallbacks const*)) noexcept;

    SharedHandle(SharedHandle const&) = default;

    SharedHandle(SharedHandle&&) noexcept = default;

    ~SharedHandle() = default;

    auto operator=(SharedHandle const&) -> SharedHandle& = default;

    auto operator=(SharedHandle&&) noexcept -> SharedHandle& = default;

    explicit operator T() const { return sharedData_.handle(); }

    explicit operator bool() const { return sharedData_.handle() != VK_NULL_HANDLE; }

    auto operator&() const -> T const* { return &sharedData_.handlePtr(); }

    auto operator&() -> T*;

    auto operator<=>(SharedHandle const& rhs) const -> intptr_t;

    auto _replace() -> T*;

    [[nodiscard]] auto unique() const -> bool { return sharedData_.unique(); }

    [[nodiscard]] auto useCount() const -> uint32_t { return sharedData_.useCount(); }

private:
    struct ControlBlock
    {
        std::atomic<uint32_t> refCount;
        std::atomic<uint32_t> weakRefCount;
        std::function<void(T, VkAllocationCallbacks const*)> deleter;
    };

    class SharedData
    {
    private:
        struct private_tag
        {};

        template<typename Deleter>
        explicit SharedData(Deleter&& deleter, [[maybe_unused]] private_tag protector) noexcept;

    public:
        SharedData() noexcept;

        explicit SharedData(void (*deleter)(T, VkAllocationCallbacks const*)) noexcept;

        SharedData(VkInstance vkInstance, void (*deleter)(VkInstance, T, VkAllocationCallbacks const*)) noexcept;

        SharedData(VkDevice vkDevice, void (*deleter)(VkDevice, T, VkAllocationCallbacks const*));

        SharedData(ControlBlock const* controlBlock, T handle) noexcept;

        SharedData(SharedData const& rhs) noexcept;

        SharedData(SharedData&& rhs) noexcept;

        ~SharedData() noexcept;

        auto operator=(SharedData const& rhs) noexcept -> SharedData&;

        auto operator=(SharedData&& rhs) noexcept -> SharedData&;

        void reset();

        void destroy();

        [[nodiscard]] auto handle() const -> T { return handle_; }

        [[nodiscard]] auto handlePtr() const -> T const* { return &handle_; }

        auto handlePtr() -> T* { return &handle_; }

        [[nodiscard]] auto unique() const -> bool;

        [[nodiscard]] auto useCount() const -> uint32_t;

        [[nodiscard]] auto controlBlock() const -> ControlBlock const* { return controlBlock_; }

        auto controlBlock() -> ControlBlock* { return controlBlock_; }

    private:
        ControlBlock* controlBlock_;
        T handle_;
    };

private:
    explicit SharedHandle(SharedData const& sharedData) noexcept;

private:
    SharedData sharedData_;
};

template<typename T>
template<typename Deleter>
SharedHandle<T>::SharedData::SharedData(Deleter&& deleter, [[maybe_unused]] private_tag protector) noexcept
  : controlBlock_{ new ControlBlock{ 1, 1, std::forward<Deleter>(deleter) } }
  , handle_{ VK_NULL_HANDLE }
{}

template<typename T>
SharedHandle<T>::SharedData::SharedData() noexcept
  : SharedData{ [](T, VkAllocationCallbacks const*) -> void {} }
{}

template<typename T>
SharedHandle<T>::SharedData::SharedData(void (*deleter)(T, VkAllocationCallbacks const*)) noexcept
  : SharedData{ deleter, private_tag{} }
{}

template<typename T>
SharedHandle<T>::SharedData::SharedData(VkInstance vkInstance,
                                        void (*deleter)(VkInstance, T, VkAllocationCallbacks const*)) noexcept
  : SharedData{ [deleter, vkInstance](T handle, VkAllocationCallbacks const* vkAllocationCallbacks) -> void {
                   deleter(vkInstance, handle, vkAllocationCallbacks);
               },
                private_tag{} }
{}

template<typename T>
SharedHandle<T>::SharedData::SharedData(VkDevice vkDevice, void (*deleter)(VkDevice, T, VkAllocationCallbacks const*))
  : SharedData(
      [deleter, vkDevice](T handle, VkAllocationCallbacks const* vkAllocationCallbacks) -> void {
          deleter(vkDevice, handle, vkAllocationCallbacks);
      },
      private_tag{})
{}

template<typename T>
SharedHandle<T>::SharedData::SharedData(ControlBlock const* controlBlock, T handle) noexcept
  : controlBlock_{ controlBlock }
  , handle_{ handle }
{
    if (controlBlock_ != nullptr)
        controlBlock_->refCount.fetch_add(1, std::memory_order_relaxed);
}

template<typename T>
SharedHandle<T>::SharedData::SharedData(SharedData const& rhs) noexcept
  : controlBlock_{ rhs.controlBlock_ }
  , handle_{ rhs.handle_ }
{
    if (controlBlock_ != nullptr)
        controlBlock_->refCount.fetch_add(1, std::memory_order_relaxed);
}

template<typename T>
SharedHandle<T>::SharedData::SharedData(SharedData&& rhs) noexcept
  : controlBlock_{ rhs.controlBlock_ }
  , handle_{ rhs.handle_ }
{
    rhs.controlBlock_ = nullptr;
    rhs.handle_ = VK_NULL_HANDLE;
}

template<typename T>
SharedHandle<T>::SharedData::~SharedData() noexcept
{
    if (controlBlock_ != nullptr) {
        if (controlBlock_->refCount.fetch_sub(1, std::memory_order_acq_rel) == 1) {
            reset();

            if (controlBlock_->weakRefCount.fetch_sub(1, std::memory_order_acq_rel) == 1) {
                destroy();
            }
        }
    }
}

template<typename T>
auto SharedHandle<T>::SharedData::operator=(SharedData const& rhs) noexcept -> SharedData&
{
    auto* tmp = rhs.controlBlock_;

    if (tmp != controlBlock_) {
        if (tmp != nullptr) {
            tmp->refCount.fetch_add(1, std::memory_order_relaxed);
        }
        if (controlBlock_ != nullptr) {
            if (controlBlock_->refCount.fetch_sub(1, std::memory_order_acq_rel) == 1) {
                reset();

                if (controlBlock_->weakRefCount.fetch_sub(1, std::memory_order_acq_rel) == 1) {
                    destroy();
                }
            }
        }

        controlBlock_ = tmp;
        handle_ = rhs.handle_;
    }

    return *this;
}

template<typename T>
auto SharedHandle<T>::SharedData::operator=(SharedData&& rhs) noexcept -> SharedData&
{
    auto* tmp = rhs.controlBlock_;
    auto handle = rhs.handle_;

    rhs.controlBlock_ = nullptr;
    rhs.handle_ = VK_NULL_HANDLE;

    if (controlBlock_ != nullptr) {
        if (controlBlock_->refCount.fetch_sub(1, std::memory_order_acq_rel) == 1) {
            reset();

            if (controlBlock_->weakRefCount.fetch_sub(1, std::memory_order_acq_rel) == 1) {
                destroy();
            }
        }
    }

    controlBlock_ = tmp;
    handle_ = handle;

    return *this;
}

template<typename T>
void SharedHandle<T>::SharedData::reset()
{
    if (handle_ != VK_NULL_HANDLE) {
        controlBlock_->deleter(handle_, nullptr);
        handle_ = VK_NULL_HANDLE;
    }
}

template<typename T>
void SharedHandle<T>::SharedData::destroy()
{
    assert(controlBlock_ != nullptr);

    delete controlBlock_;
    controlBlock_ = nullptr;
}

template<typename T>
auto SharedHandle<T>::SharedData::unique() const -> bool
{
    if (controlBlock_ == nullptr)
        return true;

    return (controlBlock_->refCount.load(std::memory_order_acquire) == 1);
}

template<typename T>
auto SharedHandle<T>::SharedData::useCount() const -> uint32_t
{
    if (controlBlock_ == nullptr)
        return 0;

    return controlBlock_->refCount.load(std::memory_order_acquire);
}

// SharedHandle
template<typename T>
SharedHandle<T>::SharedHandle() noexcept
  : sharedData_{}
{}

template<typename T>
SharedHandle<T>::SharedHandle(void (*deleter)(T, VkAllocationCallbacks const*)) noexcept
  : sharedData_{ deleter }
{}

template<typename T>
SharedHandle<T>::SharedHandle(VkInstance vkInstance,
                              void (*deleter)(VkInstance, T, VkAllocationCallbacks const*)) noexcept
  : sharedData_{ vkInstance, deleter }
{}

template<typename T>
SharedHandle<T>::SharedHandle(VkDevice vkDevice, void (*deleter)(VkDevice, T, VkAllocationCallbacks const*)) noexcept
  : sharedData_{ vkDevice, deleter }
{}

template<typename T>
SharedHandle<T>::SharedHandle(SharedData const& sharedData) noexcept
  : sharedData_{ sharedData }
{}

template<typename T>
auto SharedHandle<T>::_replace() -> T*
{
    sharedData_.reset();
    return sharedData_.handlePtr();
}

template<typename T>
auto SharedHandle<T>::operator&() -> T*
{
    assert(sharedData_.handle() == VK_NULL_HANDLE);
    return _replace();
}

template<typename T>
auto SharedHandle<T>::operator<=>(SharedHandle const& rhs) const -> intptr_t
{
    return static_cast<intptr_t>(sharedData_.handle()) - static_cast<intptr_t>(rhs.sharedData_.handle());
}
}

#endif // CYCLONITE_SHARED_HANDLE_H
