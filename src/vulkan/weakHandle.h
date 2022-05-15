//
// Created by anton on 1/18/22.
//

#ifndef CYCLONITE_WEAKHANDLE_H
#define CYCLONITE_WEAKHANDLE_H

#include "sharedHandle.h"

namespace cyclonite::vulkan {
template<typename T>
class WeakHandle
{
public:
    WeakHandle() = default;

    WeakHandle(WeakHandle const&) = default;

    WeakHandle(WeakHandle&&) noexcept = default;

    explicit WeakHandle(SharedHandle<T>& sharedHandle) noexcept;

    explicit WeakHandle(SharedHandle<T>&& sharedHandle) noexcept;

    ~WeakHandle() = default;

    auto operator=(WeakHandle const&) -> WeakHandle& = default;

    auto operator=(WeakHandle&&) noexcept -> WeakHandle& = default;

    auto lock() -> SharedHandle<T>;

    [[nodiscard]] auto expired() const -> bool;

private:
    class SharedData
    {
    private:
        using control_block_t = typename SharedHandle<T>::ControlBlock;

    public:
        SharedData() noexcept;

        SharedData(SharedData const& sharedData) noexcept;

        SharedData(SharedData&& sharedData) noexcept;

        SharedData(control_block_t* controlBlock, T handle) noexcept;

        ~SharedData() noexcept;

        auto operator=(SharedData const& rhs) noexcept -> SharedData&;

        auto operator=(SharedData&& rhs) noexcept -> SharedData&;

        void destroy();

        auto controlBlock() -> control_block_t* { return controlBlock_; }

        [[nodiscard]] auto controlBlock() const -> control_block_t const* { return controlBlock_; }

        [[nodiscard]] auto handle() const -> T { return handle_; }

        [[nodiscard]] auto useCount() const -> uint32_t;

    private:
        control_block_t* controlBlock_;
        T handle_;
    };

    SharedData sharedData_;
};

template<typename T>
WeakHandle<T>::WeakHandle(SharedHandle<T>& sharedHandle) noexcept
  : sharedData_{ sharedHandle.sharedData_.handle(), sharedHandle.sharedData_.controlBlock() }
{}

template<typename T>
WeakHandle<T>::WeakHandle(SharedHandle<T>&& sharedHandle) noexcept
  : sharedData_{ sharedHandle.sharedData_.handle(), sharedHandle.sharedData_.controlBlock() }
{}

template<typename T>
auto WeakHandle<T>::lock() -> SharedHandle<T>
{
    return SharedHandle<T>{ SharedHandle<T>::SharedData(sharedData_.controlBlock(), sharedData_.handle()) };
}

template<typename T>
auto WeakHandle<T>::expired() const -> bool
{
    return sharedData_.useCount() == 0;
}

template<typename T>
WeakHandle<T>::SharedData::SharedData() noexcept
  : controlBlock_{ nullptr }
  , handle_{ VK_NULL_HANDLE }
{}

template<typename T>
WeakHandle<T>::SharedData::SharedData(SharedData const& sharedData) noexcept
  : controlBlock_{ sharedData.controlBlock_ }
  , handle_{ sharedData.handle_ }
{
    if (controlBlock_ != nullptr)
        controlBlock_->weakRefCount.fetch_add(std::memory_order_relaxed);
}

template<typename T>
WeakHandle<T>::SharedData::SharedData(SharedData&& sharedData) noexcept
  : controlBlock_{ sharedData.controlBlock_ }
  , handle_{ sharedData.handle_ }
{
    sharedData.controlBlock_ = nullptr;
    sharedData.handle_ = VK_NULL_HANDLE;
}

template<typename T>
WeakHandle<T>::SharedData::SharedData(control_block_t* controlBlock, T handle) noexcept
  : controlBlock_{ controlBlock }
  , handle_{ handle }
{
    if (controlBlock_ != nullptr) {
        controlBlock_->weakRefCount.fetch_add(std::memory_order_relaxed);
    }
}

template<typename T>
WeakHandle<T>::SharedData::~SharedData() noexcept
{
    if (controlBlock_ != nullptr) {
        if (controlBlock_->weakRefCount.fetch_sub(std::memory_order_acq_rel) == 1) {
            destroy();
        }
    }
    handle_ = VK_NULL_HANDLE;
}

template<typename T>
auto WeakHandle<T>::SharedData::operator=(SharedData const& rhs) noexcept -> SharedData&
{
    auto* tmp = rhs.controlBlock_;

    if (tmp != controlBlock_) {
        if (tmp != nullptr) {
            tmp->weakRefCount.fetch_add(std::memory_order_relaxed);
        }

        if (controlBlock_ != nullptr) {
            if (controlBlock_->weakRefCount.fetch_sub(std::memory_order_acq_rel) == 1) {
                destroy();
            }
        }

        controlBlock_ = tmp;
        handle_ = rhs.handle_;
    }

    return *this;
}

template<typename T>
auto WeakHandle<T>::SharedData::operator=(SharedData&& rhs) noexcept -> SharedData&
{
    auto* tmp = rhs.controlBlock_;
    auto handle = rhs.handle_;

    rhs.controlBlock_ = nullptr;
    rhs.handle_ = VK_NULL_HANDLE;

    if (controlBlock_ != nullptr) {
        if (controlBlock_->weakRefCount.fetch_sub(std::memory_order_acq_rel) == 1) {
            destroy();
        }
    }

    controlBlock_ = tmp;
    handle_ = handle;

    return *this;
}

template<typename T>
void WeakHandle<T>::SharedData::destroy()
{
    assert(controlBlock_ != nullptr);

    delete controlBlock_;
    controlBlock_ = nullptr;
}

template<typename T>
auto WeakHandle<T>::SharedData::useCount() const -> uint32_t
{
    if (controlBlock_ == nullptr)
        return 0;

    return controlBlock_->refCount.load(std::memory_order_acquire);
}
}

#endif // CYCLONITE_WEAKHANDLE_H
