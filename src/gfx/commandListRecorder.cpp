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

    batchRecorder_->futures().emplace_back(multithreading::TaskManager::submitTask(task, purpose));
}

void CommandListRecorder::beginRenderPass(core::ResourceSharedRef renderPassRef)
{
    auto purpose = batchRecorder_->submission().purpose();

    auto task = [recorder = batchRecorder_, renderPassRef = std::move(renderPassRef)]() mutable -> void {
        if (!recorder->submission().isInCommandListRecordingState()) {
            throw std::runtime_error("command list recording is already finished");
        }

        auto& renderPass = renderPassRef.as<gfx::RenderPass>();
        if (renderPass.isPresentationPass()) {
            auto const& swapchainSignal = renderPass.acquireSwapchainSignal(recorder->submission().currentFrameIndex());

            recorder->addBatchDependency(gfx::SubmissionBatchDependency{
              swapchainSignal, PipelineStageFlagBits{ PipelineStageFlags::FRAGMENT_SHADER_BIT } });

            recorder->addPresentationSignal(renderPass.presentationSignal());
        }

        recorder->submission().commandListToRecord().beginRenderPass(renderPassRef);
    };

    batchRecorder_->futures().emplace_back(multithreading::TaskManager::submitTask(task, purpose));
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
        if (!recorder->submission().isInCommandListRecordingState()) {
            throw std::runtime_error("command list recording is already finished");
        }

        recorder->submission().commandListToRecord().bindDescriptorSet(
          bindPoint, bindingSchemaRef, descriptorSetRef, dynamicOffsets);
    };

    batchRecorder_->futures().emplace_back(multithreading::TaskManager::submitTask(task, purpose));
}

void CommandListRecorder::bindIndexBuffer(core::ResourceSharedRef bufferRef, size_t offset, IndexType indexType)
{
    auto purpose = batchRecorder_->submission().purpose();

    auto task = [recorder = batchRecorder_, bufferRef = std::move(bufferRef), offset, indexType]() mutable -> void {
        if (!recorder->submission().isInCommandListRecordingState()) {
            throw std::runtime_error("command list recording is already finished");
        }

        recorder->submission().commandListToRecord().bindIndexBuffer(bufferRef, offset, indexType);
    };

    batchRecorder_->futures().emplace_back(multithreading::TaskManager::submitTask(task, purpose));
}

void CommandListRecorder::endRenderPass()
{
    auto purpose = batchRecorder_->submission().purpose();

    auto task = [recorder = batchRecorder_]() -> void {
        if (!recorder->submission().isInCommandListRecordingState()) {
            throw std::runtime_error("command list recording is already finished");
        }
        recorder->submission().commandListToRecord().endRenderPass();
    };

    batchRecorder_->futures().emplace_back(multithreading::TaskManager::submitTask(task, purpose));
}

void CommandListRecorder::end()
{
    auto purpose = batchRecorder_->submission().purpose();

    auto task = [recorder = batchRecorder_]() -> void {
        if (!recorder->submission().isInCommandListRecordingState()) {
            throw std::runtime_error("command list recording is already finished");
        }
        recorder->submission().commandListToRecord().end();
    };

    batchRecorder_->futures().emplace_back(multithreading::TaskManager::submitTask(task, purpose));
}

void CommandListRecorder::bindPipeline(core::ResourceSharedRef pipeline)
{
    auto purpose = batchRecorder_->submission().purpose();

    auto task = [recorder = batchRecorder_, pipeline = std::move(pipeline)]() mutable -> void {
        if (!recorder->submission().isInCommandListRecordingState()) {
            throw std::runtime_error("command list recording is already finished");
        }
        recorder->submission().commandListToRecord().bindPipeline(pipeline);
    };

    batchRecorder_->futures().emplace_back(multithreading::TaskManager::submitTask(task, purpose));
}

void CommandListRecorder::draw(uint32_t vertexCount,
                               uint32_t instanceCount,
                               uint32_t firstVertex,
                               uint32_t firstInstance)
{
    auto purpose = batchRecorder_->submission().purpose();

    auto task = [recorder = batchRecorder_, vertexCount, instanceCount, firstVertex, firstInstance]() -> void {
        if (!recorder->submission().isInCommandListRecordingState()) {
            throw std::runtime_error("command list recording is already finished");
        }
        recorder->submission().commandListToRecord().draw(vertexCount, instanceCount, firstVertex, firstInstance);
    };

    batchRecorder_->futures().emplace_back(multithreading::TaskManager::submitTask(task, purpose));
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
        if (!recorder->submission().isInCommandListRecordingState()) {
            throw std::runtime_error("command list recording is already finished");
        }
        recorder->submission().commandListToRecord().drawIndexed(
          indexCount, instanceCount, firstIndex, vertexOffset, firstInstance);
    };

    batchRecorder_->futures().emplace_back(multithreading::TaskManager::submitTask(task, purpose));
}

void CommandListRecorder::drawIndirect(core::ResourceSharedRef bufferRef, size_t offset, uint32_t count)
{
    auto purpose = batchRecorder_->submission().purpose();

    auto task = [recorder = batchRecorder_, bufferRef = std::move(bufferRef), offset, count]() mutable -> void {
        if (!recorder->submission().isInCommandListRecordingState()) {
            throw std::runtime_error("command list recording is already finished");
        }
        recorder->submission().commandListToRecord().drawIndirect(bufferRef, offset, count);
    };

    batchRecorder_->futures().emplace_back(multithreading::TaskManager::submitTask(task, purpose));
}

void CommandListRecorder::drawIndexedIndirect(core::ResourceSharedRef bufferRef, size_t offset, uint32_t count)
{
    auto purpose = batchRecorder_->submission().purpose();

    auto task = [recorder = batchRecorder_, bufferRef = std::move(bufferRef), offset, count]() mutable -> void {
        if (!recorder->submission().isInCommandListRecordingState()) {
            throw std::runtime_error("command list recording is already finished");
        }
        recorder->submission().commandListToRecord().drawIndexedIndirect(bufferRef, offset, count);
    };

    batchRecorder_->futures().emplace_back(multithreading::TaskManager::submitTask(task, purpose));
}

void CommandListRecorder::finish(bool noexceptions)
{
    try {
        auto purpose = batchRecorder_->submission().purpose();

        auto task = [recorder = batchRecorder_]() -> void {
            if (!recorder->submission().isInCommandListRecordingState()) {
                throw std::runtime_error("command list recording is already finished");
            }

            recorder->submission().endCommandListRecording();
        };

        batchRecorder_->futures().emplace_back(multithreading::TaskManager::submitTask(task, purpose));

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
}
}
