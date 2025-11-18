//
// Created by anton on 10/12/25.
//

#ifndef CYCLONITE_COMMAND_LIST_RECORDER_H
#define CYCLONITE_COMMAND_LIST_RECORDER_H

#include "core/resourceSharedRef.h"
#include "gfx/common.h"
#include <span>

namespace cyclonite::gfx {
class SubmissionBatchRecorder;

class CommandListRecorder
{
public:
    explicit CommandListRecorder(SubmissionBatchRecorder* batchRecorder);

    ~CommandListRecorder();

    void begin(CommandListUsageFlagBits usage);

    void beginRenderPass(core::ResourceSharedRef renderPassRef);

    void endRenderPass();

    void bindPipeline(core::ResourceSharedRef pipeline);

    void bindDescriptorSet(PipelineBindPoint bindPoint,
                           core::ResourceSharedRef bindingSchemaRef,
                           core::ResourceSharedRef descriptorSetRef,
                           std::span<uint32_t> dynamicOffsets = {});

    void end();

    void finish() { finish(false); }

private:
    void finish(bool noexceptions);

    SubmissionBatchRecorder* batchRecorder_;
};
}

#endif // CYCLONITE_COMMAND_LIST_RECORDER_H