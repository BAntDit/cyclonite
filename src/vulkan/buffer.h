//
// Created by bantdit on 10/5/19.
//

#ifndef CYCLONITE_BUFFER_H
#define CYCLONITE_BUFFER_H

#include "device.h"
#include <array>
#include <variant>

namespace cyclonite::vulkan {
class Buffer
{
public:
    using owner_queue_family_indices_t =
      std::variant<std::array<uint32_t, 1>, std::array<uint32_t, 2>, std::array<uint32_t, 3>>;

    Buffer(Device const& device,
           VkMemoryPropertyFlags memoryPropertyFlags,
           VkBufferUsageFlags usageFlags,
           VkDeviceSize size,
           owner_queue_family_indices_t ownerQueueFamilyIndices);

private:
    Handle<VkBuffer> vkBuffer_;
};
}

#endif // CYCLONITE_BUFFER_H
