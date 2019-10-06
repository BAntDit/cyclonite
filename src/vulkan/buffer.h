//
// Created by bantdit on 10/5/19.
//

#ifndef CYCLONITE_BUFFER_H
#define CYCLONITE_BUFFER_H

#include "memoryPage.h"
#include <array>
#include <variant>

namespace cyclonite::vulkan {
class Buffer
{
public:
    using owner_queue_family_indices_t =
      std::variant<std::array<uint32_t, 1>, std::array<uint32_t, 2>, std::array<uint32_t, 3>>;

    Buffer(Device& device,
           VkMemoryPropertyFlags memoryPropertyFlags,
           VkBufferUsageFlags usageFlags,
           VkDeviceSize size,
           owner_queue_family_indices_t ownerQueueFamilyIndices);

    [[nodiscard]] auto handle() const -> VkBuffer { return static_cast<VkBuffer>(vkBuffer_); }

private:
    MemoryPage::AllocatedMemory allocatedMemory_;
    Handle<VkBuffer> vkBuffer_;
};
}

#endif // CYCLONITE_BUFFER_H
