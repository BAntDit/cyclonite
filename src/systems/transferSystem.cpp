//
// Created by bantdit on 2/22/20.
//

#include "transferSystem.h"

namespace cyclonite::systems {
void TransferSystem::init(vulkan::Device& device)
{
    vkDevice_ = device.handle();
    transferVersion_ = std::numeric_limits<uint32_t>::max();
}
}