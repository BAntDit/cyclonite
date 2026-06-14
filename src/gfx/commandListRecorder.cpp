//
// Created by anton on 10/12/25.
//

#include "commandListRecorder.h"
#include "gfx/renderPass.h"
#include "multithreading/taskManager.h"
#include "queueSubmissionRecorder.h"
#include "submissionBatchRecorder.h"

namespace cyclonite::gfx {
CommandListRecorder::CommandListRecorder(SubmissionBatchRecorder* batchRecorder)
  : batchRecorder_{ batchRecorder }
{
}

CommandListRecorder::~CommandListRecorder()
{
    if (batchRecorder_ != nullptr) {
        finish(true);
        batchRecorder_ = nullptr;
    }
}

void CommandListRecorder::begin(CommandListUsageFlagBits usage)
{
    auto purpose = batchRecorder_->submission().purpose();

    auto task = [recorder = batchRecorder_, usage]() -> void {
        if (!recorder->submission().isInCommandListRecordingState()) {
            throw std::runtime_error("command list recording is already finished");
        }
        recorder->submission().commandListToRecord().begin(usage);
    };

    batchRecorder_->queueSubmissionRecorder().addTask(multithreading::TaskManager::submitTask(task, purpose));
}

void CommandListRecorder::beginRenderPass(core::ResourceSharedRef renderPassRef)
{
    auto purpose = batchRecorder_->submission().purpose();

    auto task = [recorder = batchRecorder_, renderPassRef = std::move(renderPassRef)]() mutable -> void {
        auto& submission = recorder->submission();
        if (!submission.isInCommandListRecordingState()) {
            throw std::runtime_error("command list recording is already finished");
        }

        auto& renderPass = renderPassRef.as<gfx::RenderPass>();
        if (renderPass.isPresentationPass()) {
            auto const& swapchainSignal = renderPass.acquireSwapchainSignal(submission.currentFrameIndex());

            recorder->addBatchDependency(gfx::SubmissionBatchDependency{
              swapchainSignal, PipelineStageFlagBits{ PipelineStageFlags::FRAGMENT_SHADER_BIT } });

            recorder->addPresentationSignal(renderPass.presentationSignal());
        }

        submission.commandListToRecord().beginRenderPass(renderPassRef);
    };

    batchRecorder_->queueSubmissionRecorder().addTask(multithreading::TaskManager::submitTask(task, purpose));
}

void CommandListRecorder::bindDescriptorSet(PipelineBindPoint bindPoint,
                                            core::ResourceSharedRef bindingSchemaRef,
                                            core::ResourceSharedRef descriptorSetRef,
                                            std::span<uint32_t> dynamicOffsets)
{
    auto purpose = batchRecorder_->submission().purpose();

    auto task = [recorder = batchRecorder_,
                 bindPoint,
                 bindingSchemaRef = std::move(bindingSchemaRef),
                 descriptorSetRef = std::move(descriptorSetRef),
                 dynamicOffsets]() mutable -> void {
        auto& submission = recorder->submission();
        if (!submission.isInCommandListRecordingState()) {
            throw std::runtime_error("command list recording is already finished");
        }

        submission.commandListToRecord().bindDescriptorSet(
          bindPoint, bindingSchemaRef, descriptorSetRef, dynamicOffsets);
    };

    batchRecorder_->queueSubmissionRecorder().addTask(multithreading::TaskManager::submitTask(task, purpose));
}

void CommandListRecorder::bindIndexBuffer(core::ResourceSharedRef bufferRef, size_t offset, IndexType indexType)
{
    auto purpose = batchRecorder_->submission().purpose();

    auto task = [recorder = batchRecorder_, bufferRef = std::move(bufferRef), offset, indexType]() mutable -> void {
        auto& submission = recorder->submission();
        if (!submission.isInCommandListRecordingState()) {
            throw std::runtime_error("command list recording is already finished");
        }

        submission.commandListToRecord().bindIndexBuffer(bufferRef, offset, indexType);
    };

    batchRecorder_->queueSubmissionRecorder().addTask(multithreading::TaskManager::submitTask(task, purpose));
}

void CommandListRecorder::endRenderPass()
{
    auto purpose = batchRecorder_->submission().purpose();

    auto task = [recorder = batchRecorder_]() -> void {
        auto& submission = recorder->submission();
        if (!submission.isInCommandListRecordingState()) {
            throw std::runtime_error("command list recording is already finished");
        }
        submission.commandListToRecord().endRenderPass();
    };

    batchRecorder_->queueSubmissionRecorder().addTask(multithreading::TaskManager::submitTask(task, purpose));
}

void CommandListRecorder::bufferMemoryBarrier(PipelineStageFlagBits srcStageMask,
                                              PipelineStageFlagBits dstStageMask,
                                              AccessFlagBits srcAccessMask,
                                              AccessFlagBits dstAccessMask,
                                              uint32_t srcQueueFamilyIndex,
                                              uint32_t dstQueueFamilyIndex,
                                              core::ResourceSharedRef const& bufferRef,
                                              size_t offset /* = 0*/,
                                              size_t size /* = std::numeric_limits<size_t>::max()*/)
{
    auto purpose = batchRecorder_->submission().purpose();

    auto task = [recorder = batchRecorder_,
                 srcStageMask,
                 dstStageMask,
                 srcAccessMask,
                 dstAccessMask,
                 srcQueueFamilyIndex,
                 dstQueueFamilyIndex,
                 bufferRef = bufferRef,
                 offset,
                 size]() -> void {
        auto& submission = recorder->submission();
        if (!submission.isInCommandListRecordingState()) {
            throw std::runtime_error("command list recording is already finished");
        }
        submission.commandListToRecord().bufferMemoryBarrier(srcStageMask,
                                                             dstStageMask,
                                                             srcAccessMask,
                                                             dstAccessMask,
                                                             srcQueueFamilyIndex,
                                                             dstQueueFamilyIndex,
                                                             bufferRef,
                                                             offset,
                                                             size);
    };

    batchRecorder_->queueSubmissionRecorder().addTask(multithreading::TaskManager::submitTask(task, purpose));
}

void CommandListRecorder::end()
{
    auto purpose = batchRecorder_->submission().purpose();

    auto task = [recorder = batchRecorder_]() -> void {
        auto& submission = recorder->submission();
        if (!submission.isInCommandListRecordingState()) {
            throw std::runtime_error("command list recording is already finished");
        }
        submission.commandListToRecord().end();
    };

    batchRecorder_->queueSubmissionRecorder().addTask(multithreading::TaskManager::submitTask(task, purpose));
}

void CommandListRecorder::bindPipeline(core::ResourceSharedRef pipeline)
{
    auto purpose = batchRecorder_->submission().purpose();

    auto task = [recorder = batchRecorder_, pipeline = std::move(pipeline)]() mutable -> void {
        auto& submission = recorder->submission();
        if (!submission.isInCommandListRecordingState()) {
            throw std::runtime_error("command list recording is already finished");
        }
        submission.commandListToRecord().bindPipeline(pipeline);
    };

    batchRecorder_->queueSubmissionRecorder().addTask(multithreading::TaskManager::submitTask(task, purpose));
}

void CommandListRecorder::draw(uint32_t vertexCount,
                               uint32_t instanceCount,
                               uint32_t firstVertex,
                               uint32_t firstInstance)
{
    auto purpose = batchRecorder_->submission().purpose();

    auto task = [recorder = batchRecorder_, vertexCount, instanceCount, firstVertex, firstInstance]() -> void {
        auto& submission = recorder->submission();
        if (!submission.isInCommandListRecordingState()) {
            throw std::runtime_error("command list recording is already finished");
        }
        submission.commandListToRecord().draw(vertexCount, instanceCount, firstVertex, firstInstance);
    };

    batchRecorder_->queueSubmissionRecorder().addTask(multithreading::TaskManager::submitTask(task, purpose));
}

void CommandListRecorder::drawIndexed(uint32_t indexCount,
                                      uint32_t instanceCount,
                                      uint32_t firstIndex,
                                      int32_t vertexOffset,
                                      uint32_t firstInstance)
{
    auto purpose = batchRecorder_->submission().purpose();

    auto task =
      [recorder = batchRecorder_, indexCount, instanceCount, firstIndex, vertexOffset, firstInstance]() -> void {
        auto& submission = recorder->submission();
        if (!submission.isInCommandListRecordingState()) {
            throw std::runtime_error("command list recording is already finished");
        }
        submission.commandListToRecord().drawIndexed(
          indexCount, instanceCount, firstIndex, vertexOffset, firstInstance);
    };

    batchRecorder_->queueSubmissionRecorder().addTask(multithreading::TaskManager::submitTask(task, purpose));
}

void CommandListRecorder::drawIndirect(core::ResourceSharedRef bufferRef, size_t offset, uint32_t count)
{
    auto purpose = batchRecorder_->submission().purpose();

    auto task = [recorder = batchRecorder_, bufferRef = std::move(bufferRef), offset, count]() mutable -> void {
        auto& submission = recorder->submission();
        if (!submission.isInCommandListRecordingState()) {
            throw std::runtime_error("command list recording is already finished");
        }
        submission.commandListToRecord().drawIndirect(bufferRef, offset, count);
    };

    batchRecorder_->queueSubmissionRecorder().addTask(multithreading::TaskManager::submitTask(task, purpose));
}

void CommandListRecorder::drawIndexedIndirect(core::ResourceSharedRef bufferRef, size_t offset, uint32_t count)
{
    auto purpose = batchRecorder_->submission().purpose();

    auto task = [recorder = batchRecorder_, bufferRef = std::move(bufferRef), offset, count]() mutable -> void {
        auto& submission = recorder->submission();
        if (!submission.isInCommandListRecordingState()) {
            throw std::runtime_error("command list recording is already finished");
        }
        submission.commandListToRecord().drawIndexedIndirect(bufferRef, offset, count);
    };

    batchRecorder_->queueSubmissionRecorder().addTask(multithreading::TaskManager::submitTask(task, purpose));
}

void CommandListRecorder::finish(bool noexceptions)
{
    try {
        auto purpose = batchRecorder_->submission().purpose();

        auto task = [recorder = batchRecorder_]() -> void {
            auto& submission = recorder->submission();
            if (!submission.isInCommandListRecordingState()) {
                throw std::runtime_error("command list recording is already finished");
            }

            submission.endCommandListRecording();
        };

        batchRecorder_->queueSubmissionRecorder().addTask(multithreading::TaskManager::submitTask(task, purpose));

        if (auto ex = multithreading::Executor::threadExecutor().taskManager().getLastException(); ex) {
            std::rethrow_exception(ex);
        }
    } catch (...) {
        if (noexceptions) {
            batchRecorder_->submission().reset();
            return;
        } else {
            auto ex = std::current_exception();

            std::rethrow_exception(ex);
        }
    }
    batchRecorder_ = nullptr;
}
}
