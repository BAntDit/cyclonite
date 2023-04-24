//
// Created by anton on 3/20/23.
//

#include "buffers/ring.h"
#include "shaderResource.h"
#include "vulkan/buffer.h"
#include <vector>

#ifndef CYCLONITE_UNIFORMSTAGING_H
#define CYCLONITE_UNIFORMSTAGING_H

namespace cyclonite {
class Root;
}

namespace cyclonite::render {
class ConstantBufferManager : public buffers::Ring<ConstantBufferManager>
{
public:
    // TODO::
    ConstantBufferManager(Root& root, size_t size);

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
