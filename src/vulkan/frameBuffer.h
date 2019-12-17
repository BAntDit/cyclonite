//
// Created by bantdit on 12/17/19.
//

#ifndef CYCLONITE_FRAMEBUFFER_H
#define CYCLONITE_FRAMEBUFFER_H

#include <easy-mp/type_list.h>
#include "device.h"
#include "imageView.h"

namespace cyclonite::vulkan {

template<typename... Args>
auto create_image_view(std::tuple<Args&&...>&& params) -> ImageView {
    return vulkan::ImageView{ std::get<std::forward<Args>>(params)... };
}

template<typename count, typename args>
struct create_image_views;

template<typename... args, size_t... indices>
struct create_image_views<std::index_sequence<indices...>, easy_mp::type_list<args...>>
{
    static auto _create(std::array<std::tuple<args&&...>&&, sizeof...(indices)>&& params) -> std::array<ImageView, sizeof...(indices)> {
        return std::array<ImageView, sizeof...(indices)>{ create_image_view(std::forward(params[indices]))... };
    }
};

template<size_t attachmentCount>
class FrameBuffer
{
public:
    FrameBuffer(vulkan::Device const& device,
                VkRenderPass vkRenderPass,
                uint32_t width,
                uint32_t height,
                std::array<vulkan::ImageView, attachmentCount>&& attachments);

    template<typename... ImageViewArg>
    FrameBuffer(vulkan::Device const& device,
                VkRenderPass vkRenderPass,
                uint32_t width,
                uint32_t height,
                std::array<std::tuple<ImageViewArg...>, attachmentCount>&& attachmentArgs);

    FrameBuffer(FrameBuffer const&) = delete;

    FrameBuffer(FrameBuffer&&) = default;

    ~FrameBuffer() = default;

    auto operator=(FrameBuffer const&) -> FrameBuffer& = delete;

    auto operator=(FrameBuffer &&) -> FrameBuffer& = default;

    [[nodiscard]] auto handle() const -> VkFramebuffer { return static_cast<VkFramebuffer>(vkFrameBuffer_); }

    [[nodiscard]] auto getAttachment(size_t attachmentIndex) const -> ImageView const&
    {
        assert(attachmentIndex < attachmentCount);
        return attachments_[attachmentIndex];
    }

private:
    std::array<vulkan::ImageView, attachmentCount> attachments_;
    vulkan::Handle<VkFramebuffer> vkFrameBuffer_;
};

template<size_t attachmentCount>
FrameBuffer<attachmentCount>::FrameBuffer(vulkan::Device const& device,
                                          VkRenderPass vkRenderPass,
                                          uint32_t width,
                                          uint32_t height,
                                          std::array<vulkan::ImageView, attachmentCount>&& attachments)
  : attachments_{ std::move(attachments) }
  , vkFrameBuffer_{ device.handle(), vkDestroyFramebuffer }
{
    VkFramebufferCreateInfo framebufferInfo = {};

    framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    framebufferInfo.renderPass = vkRenderPass;
    framebufferInfo.attachmentCount = static_cast<uint32_t>(attachments_.size());
    framebufferInfo.pAttachments = attachments_.data();
    framebufferInfo.width = width;
    framebufferInfo.height = height;
    framebufferInfo.layers = 1; // for now

    if (auto result = vkCreateFramebuffer(device.handle(), &framebufferInfo, nullptr, &vkFrameBuffer_);
        result != VK_SUCCESS) {
        if (result == VK_ERROR_OUT_OF_HOST_MEMORY) {
            throw std::runtime_error("failed to create framebuffer: out of host memory");
        }

        if (result == VK_ERROR_OUT_OF_DEVICE_MEMORY) {
            throw std::runtime_error("failed to create framebuffer: out of device memory");
        }

        assert(false);
    }
}
}

/*
 * #include <cstdint>
#include <array>
#include <tuple>

template<typename... Ts>
struct type_list {};

struct foo {
    foo(uint32_t, uint16_t) {}

    explicit foo(size_t) {}
};

template<typename... Args>
auto create_foo(std::tuple<Args...> params) -> foo {
    return foo{ std::get<Args>(params)... };
}

template<typename count, typename args>
struct create_many_foo;

template<typename... args, size_t... indices>
struct create_many_foo<std::index_sequence<indices...>, type_list<args...>>
{
    static auto _create(std::array<std::tuple<args...>, sizeof...(indices)>&& params) -> std::array<foo, sizeof...(indices)> {
        return std::array<foo, sizeof...(indices)>{ create_foo(params[indices])... };
    }
};

template<size_t Count>
struct Bar {
    Bar(std::array<foo, Count>&& manyFoo) : manyFoo_{ std::move(manyFoo) } {}

    template<typename... Args>
    Bar(std::array<std::tuple<Args...>, Count>&& argSets) :
        Bar{ create_many_foo<std::make_index_sequence<Count>, type_list<Args...>>::_create(std::move(argSets)) }
    {}

    std::array<foo, Count> manyFoo_;
};

int main() {
    Bar<2> bar{ std::array<std::tuple<uint32_t, uint16_t>, 2>{
            std::make_tuple(static_cast<uint32_t>(0), static_cast<uint16_t>(1)),
            std::make_tuple(static_cast<uint32_t>(0), static_cast<uint16_t>(2))
        } };
}
 */

#endif // CYCLONITE_FRAMEBUFFER_H
