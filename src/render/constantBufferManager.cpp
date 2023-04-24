//
// Created by anton on 4/24/23.
//

#include "constantBufferManager.h"
#include "root.h"

namespace cyclonite::render {
ConstantBufferManager::ConstantBufferManager(Root& root, size_t size)
  : buffers::Ring<ConstantBufferManager>{ size, root.capabilities().minUniformBufferOffsetAlignment }
  , ranges_{} // TODO:: adds initial ranges count and free indices storage
  , staging_{ root.device(),
              VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
              VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
              size,
              std::array{ root.device().hostTransferQueueFamilyIndex() } }
  , buffer_{ root.device(),
             VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
             VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
             size,
             std::array{ root.device().hostTransferQueueFamilyIndex(), root.device().graphicsQueueFamilyIndex() } }
{
}
}