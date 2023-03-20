//
// Created by anton on 3/20/23.
//

#include "buffers/ring.h"
#include "vulkan/buffer.h"
#include "shaderResource.h"
#include <vector>

#ifndef CYCLONITE_UNIFORMSTAGING_H
#define CYCLONITE_UNIFORMSTAGING_H

namespace cyclonite::render {
class ConstantBufferManager : public buffers::Ring<ConstantBufferManager>
{
public:
    // TODO::
    ConstantBufferManager(size_t size, size_t frameCount);

    void beginFrame(size_t index, VkFence);

    auto allocConstantBuffer(size_t size) -> ShaderResourceUBODescriptor;

    void endFrame(size_t index);

private:
    struct range_t
    {
        VkFence fence;
        size_t from;
        size_t to;
    };

    std::vector<range_t> ranges_;
    vulkan::Buffer staging_;
    vulkan::Buffer buffer_;
};
}

#endif // CYCLONITE_UNIFORMSTAGING_H
