//
// Created by bantdit on 9/27/19.
//

#ifndef CYCLONITE_MEMORYMANAGER_H
#define CYCLONITE_MEMORYMANAGER_H

#include "../multithreading/taskManager.h"
#include "device.h"

namespace cyclonite::vulkan {
class MemoryManager
{
public:
    MemoryManager(multithreading::TaskManager const& taskManager, Device const& device);

private:
    multithreading::TaskManager const* taskManager_;
};
}

#endif // CYCLONITE_MEMORYMANAGER_H
