//
// Created by anton on 10/12/25.
//

#ifndef CYCLONITE_COMMAND_LIST_RECORDER_H
#define CYCLONITE_COMMAND_LIST_RECORDER_H

#include "core/resourceSharedRef.h"
#include "gfx/common.h"
#include <span>

#include "frameRecordingContext.h"

namespace cyclonite::gfx {
class SubmissionBatchRecorder;
class FrameRecordingContext;

class CommandListRecorder
{
public:
    explicit CommandListRecorder(SubmissionBatchRecorder* batchRecorder, FrameRecordingContext* recordingContext);

    ~CommandListRecorder();

    void begin(CommandListUsageFlagBits usage);

    void beginRenderPass(core::ResourceSharedRef renderPassRef);

    void endRenderPass();

    void bindPipeline(core::ResourceSharedRef pipeline);

    void bindDescriptorSet(PipelineBindPoint bindPoint,
                           core::ResourceSharedRef bindingSchemaRef,
                           core::ResourceSharedRef descriptorSetRef,
                           std::span<uint32_t> dynamicOffsets = {});

    void bindIndexBuffer(core::ResourceSharedRef bufferRef, size_t offset, IndexType indexType);

    void draw(uint32_t vertexCount, uint32_t instanceCount, uint32_t firstVertex, uint32_t firstInstance);

    void drawIndexed(uint32_t indexCount,
                     uint32_t instanceCount,
                     uint32_t firstIndex,
                     int32_t vertexOffset,
                     uint32_t firstInstance);

    void drawIndirect(core::ResourceSharedRef bufferRef, size_t offset, uint32_t count);

    void drawIndexedIndirect(core::ResourceSharedRef bufferRef, size_t offset, uint32_t count);

    void end();

    void finish() { finish(false); }

private:
    void finish(bool noexceptions);

    SubmissionBatchRecorder* batchRecorder_;
    FrameRecordingContext* recordingContext_;
};
}

#endif // CYCLONITE_COMMAND_LIST_RECORDER_H