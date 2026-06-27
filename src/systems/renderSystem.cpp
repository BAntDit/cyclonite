//
// Created by anton on 5/25/26.
//

#include "renderSystem.h"
#include "gfx/device.h"
#include "gfx/queueSubmissionRecorder.h"
#include <array>
#include <span>

#include "gfx/buffer.h"
#include "gfx/descriptorSet.h"
#include "gfx/pipeline.h"
#include "gfx/renderPassBuilder.h"
#include "material.h"
#include "multithreading/taskManager.h"

namespace cyclonite::systems {
void Renderer::init(core::ResourceSharedRef const& deviceRef,
                    core::ResourceSharedRef const& renderWindowRef,
                    core::ResourceSharedRef const& vertexShaderRef,
                    core::ResourceSharedRef const& pixelShaderRef,
                    core::ResourceSharedRef const& materialRef,
                    gfx::QueueSubmissionManager& queueSubmissionManager)
{
    queueSubmissionManager_ = &queueSubmissionManager;
    deviceRef_ = deviceRef;
    renderWindowRef_ = renderWindowRef;
    materialRef_ = materialRef;

    auto& device = deviceRef_.as<gfx::Device>();

    auto allShaderStages = gfx::ShaderStageFlagBits{ gfx::ShaderStageFlags::ALL };
    auto setLayoutFlags = gfx::DescriptorSetLayoutFlagBits{ gfx::DescriptorSetLayoutFlags::UPDATE_AFTER_BIND };
    auto bindingFlags =
      gfx::BindingFlagBits{ gfx::BindingFlags::UPDATE_AFTER_BIND, gfx::BindingFlags::PARTIALLY_BOUND };

    auto bindings = std::array{ gfx::Binding{ metrix::value_cast(gfx::DescriptorSpace::PER_PASS),
                                              0,
                                              gfx::DescriptorType::UNIFORM_BUFFER,
                                              1,
                                              allShaderStages,
                                              setLayoutFlags,
                                              bindingFlags } };

    passBindingSchemaRef_ = device.getOrCreatePipelineBindingSchema(bindings, std::span<gfx::PushConstantRange>{});

    auto renderPassBuilder = cyclonite::gfx::RenderPassBuilder{};

    renderPassRef_ = cyclonite::core::ResourceSharedRef{ renderPassBuilder.setDevice(deviceRef_)
                                                           .setRenderWindow(renderWindowRef_,
                                                                            cyclonite::gfx::Color{ 0.f, 1.f, 0.f, 1.f })
                                                           .build() };

    auto shaderSet = cyclonite::Material::shader_set_t{};
    shaderSet.add(vertexShaderRef, cyclonite::gfx::ShaderStageFlags::VERTEX);
    shaderSet.add(pixelShaderRef, cyclonite::gfx::ShaderStageFlags::FRAGMENT);

    auto rasterizationState = cyclonite::gfx::RasterizationState{};
    rasterizationState.flags.reset(cyclonite::gfx::RasterizationStateFlags::RASTERIZER_DISCARD_ENABLE);

    auto& material = materialRef_.as<cyclonite::Material>();

    material.manualSetup(renderPassRef_, shaderSet, rasterizationState).get();
}

void Renderer::setupPassConstants(components::Transform const& transform, components::Camera const& camera)
{
    auto& device = deviceRef_.as<gfx::Device>();
    auto [stagingRef, stagingMap] = getPassConstantStaging();

    auto view = glm::inverse(transform.worldMatrix);
    stagingMap->view = view;

    auto projection = std::visit(
      [](auto&& projection) -> mat4 {
          if constexpr (std::is_same_v<std::decay_t<decltype(projection)>, components::Camera::PerspectiveProjection>) {
              auto& [aspect, yFov, zNear, zFar] = projection;

              real f = 1.0f / tan(0.5f * yFov);

              return glm::transpose(mat4{ f / aspect,
                                          0.f,
                                          0.f,
                                          0.f,
                                          0.f,
                                          -f,
                                          0.f,
                                          0.f,
                                          0.f,
                                          0.f,
                                          zFar / (zNear - zFar),
                                          -1.f,
                                          0.f,
                                          0.f,
                                          (zNear * zFar) / (zNear - zFar),
                                          0.f });
          }

          if constexpr (std::is_same_v<std::decay_t<decltype(projection)>,
                                       components::Camera::OrthographicProjection>) {
              auto& [xMag, yMag, zNear, zFar] = projection;
              return glm::ortho(0.0f, xMag, 0.0f, yMag, zNear, zFar);
          }
          assert(false);
      },
      camera.projection);

    stagingMap->projection = projection;

    stagingMap->viewProj = glm::transpose(projection) * view;
}

auto Renderer::getPassDescriptorSet() -> core::ResourceSharedRef
{
    auto result = core::ResourceSharedRef();
    auto submissionIndex = queueSubmissionManager_->currentSubmissionIndex();
    auto& device = deviceRef_.as<gfx::Device>();

    if (auto it = passDescriptorSets_.find(submissionIndex); it != passDescriptorSets_.end()) {
        auto const& [_, ds] = *it;
        result = ds;
    } else {
        auto ds = device.allocateDescriptorSetBySchema(
          passBindingSchemaRef_, metrix::value_cast(gfx::DescriptorSpace::PER_PASS), false);
        result = core::ResourceSharedRef{ std::move(ds) };

        [[maybe_unused]] auto [itNew, success] = passDescriptorSets_.add(result, submissionIndex);
        assert(success);
    }

    return result;
}

auto Renderer::getPassConstantStaging() -> std::pair<core::ResourceSharedRef, PassConstants*>
{
    auto submissionIndex = queueSubmissionManager_->currentSubmissionIndex();
    auto& device = deviceRef_.as<gfx::Device>();

    auto bufferRef = core::ResourceSharedRef();
    auto staging = std::add_pointer_t<PassConstants>{ nullptr };

    if (auto it = passConstantStagings_.find(submissionIndex); it != passConstantStagings_.end()) {
        auto const& [_, pair] = *it;
        bufferRef = pair.first;
        staging = pair.second;
    } else {
        auto bufferNewRef = device.createBuffer(
          gfx::GpuMemoryAllocationFlagBits{ gfx::GpuMemoryAllocationFlags::HOST_ACCESS_SEQUENTIAL_WRITE },
          gfx::BufferUsageFlagBits{ gfx::BufferUsageFlags::TRANSFER_SRC },
          sizeof(PassConstants));

        bufferRef = core::ResourceSharedRef{ std::move(bufferNewRef) };
        auto& buffer = bufferRef.as<gfx::Buffer>();

        staging = reinterpret_cast<PassConstants*>(buffer.map());

        [[maybe_unused]] auto [itNew, success] =
          passConstantStagings_.add(std::make_pair(bufferRef, staging), submissionIndex);
        assert(success);
    }

    return std::make_pair(bufferRef, staging);
}

auto Renderer::getPassConstantBuffer() -> core::ResourceSharedRef
{
    auto result = core::ResourceSharedRef();
    auto submissionIndex = queueSubmissionManager_->currentSubmissionIndex();
    auto& device = deviceRef_.as<gfx::Device>();

    if (auto it = passConstantBuffers_.find(submissionIndex); it != passConstantBuffers_.end()) {
        auto const& [_, ds] = *it;
        result = ds;
    } else {
        auto bufferNewRef = device.createBuffer(
          gfx::GpuMemoryAllocationFlagBits{ gfx::GpuMemoryAllocationFlags::DEDICATED_MEMORY },
          gfx::BufferUsageFlagBits{ gfx::BufferUsageFlags::UNIFORM_BUFFER, gfx::BufferUsageFlags::TRANSFER_DST },
          sizeof(PassConstants));

        result = core::ResourceSharedRef{ std::move(bufferNewRef) };

        [[maybe_unused]] auto [itNew, success] = passConstantBuffers_.add(result, submissionIndex);
        assert(success);
    }

    return result;
}

struct PassConstantsTransferJob
{
    explicit PassConstantsTransferJob(Renderer* renderer)
      : renderer_{ renderer } {};

    void operator()()
    {
        auto* submissionManager = renderer_->queueSubmissionManager_;

        renderer_->transferSubmission_ = submissionManager->acquireQueueSubmission(
          multithreading::Purpose::Transfer, gfx::CommandPoolFlagBits{}, gfx::default_transfer_submission_priority_v);

        auto& transferSubmission = renderer_->transferSubmission_.as<gfx::QueueSubmission>();

        auto passConstantsBufferRef = renderer_->getPassConstantBuffer();
        auto [passStagingRef, _] = renderer_->getPassConstantStaging();

        if (!transferSubmission.isExecutable()) {
            auto transferSubmissionRecorder = cyclonite::gfx::QueueSubmissionRecorder{ renderer_->transferSubmission_ };

            transferSubmissionRecorder.start();

            {
                auto batchRecorder = transferSubmissionRecorder.addBatch("pass-constants-transfer");

                auto commandListRecorder = batchRecorder.addCommandList();
                commandListRecorder.begin(gfx::CommandListUsageFlagBits{});

                // acquiring barrier
                commandListRecorder.acquireResourceForTransfer(
                  gfx::PipelineStageFlagBits{ gfx::PipelineStageFlags::TOP_OF_PIPE_BIT },
                  gfx::PipelineStageFlagBits{ gfx::PipelineStageFlags::TRANSFER_BIT },
                  gfx::AccessFlagBits{},
                  gfx::AccessFlagBits{ gfx::AccessFlags::TRANSFER_WRITE_BIT },
                  passConstantsBufferRef);

                commandListRecorder.copyBuffers(
                  passStagingRef, passConstantsBufferRef, 0, 0, sizeof(Renderer::PassConstants));

                // release barrier
                commandListRecorder.releaseResourceToGraphics(
                  gfx::PipelineStageFlagBits{ gfx::PipelineStageFlags::TRANSFER_BIT },
                  gfx::PipelineStageFlagBits{ gfx::PipelineStageFlags::BOTTOM_OF_PIPE_BIT },
                  gfx::AccessFlagBits{ gfx::AccessFlags::TRANSFER_WRITE_BIT },
                  gfx::AccessFlagBits{},
                  passConstantsBufferRef);

                commandListRecorder.end();

                commandListRecorder.finish(); // TODO:: combine with end

                batchRecorder.finish();
            }

            transferSubmissionRecorder.finish().get();
        }
    }

    Renderer* renderer_;
};

struct PassRenderJob
{
    explicit PassRenderJob(Renderer* renderer)
      : renderer_{ renderer } {};

    void operator()()
    {
        auto* submissionManager = renderer_->queueSubmissionManager_;

        auto descriptorSetRef = renderer_->getPassDescriptorSet();
        auto& descriptorSet = descriptorSetRef.as<gfx::DescriptorSet>();

        auto renderSubmissionRef = submissionManager->acquireQueueSubmission(
          multithreading::Purpose::Render,
          gfx::CommandPoolFlagBits{ cyclonite::gfx::CommandPoolFlags::TRANSIENT },
          gfx::default_render_submission_priority_v);

        auto& renderSubmission = renderSubmissionRef.as<gfx::QueueSubmission>();

        auto renderSubmissionRecorder = cyclonite::gfx::QueueSubmissionRecorder{ renderSubmissionRef };

        renderSubmissionRecorder.start();

        {
            auto batchRecorder = renderSubmissionRecorder.addBatch("test-render-pass");
            auto commandListRecorder = batchRecorder.addCommandList();

            commandListRecorder.begin(cyclonite::gfx::CommandListUsageFlagBits{});

            commandListRecorder.beginRenderPass(renderer_->renderPassRef_);

            auto& material = renderer_->materialRef_.as<cyclonite::Material>();
            commandListRecorder.bindPipeline(material.pipeline());

            renderer_->transferTaskFuture_.get();

            auto constantBufferRef = renderer_->getPassConstantBuffer();

            auto resDesc = gfx::BufferResourceDescription{};
            resDesc.type = gfx::DescriptorType::UNIFORM_BUFFER;
            resDesc.size = sizeof(Renderer::PassConstants);
            resDesc.offset = 0;

            auto descriptorWriteData = gfx::DescriptorWriteData{};
            descriptorWriteData.binding = 0;
            descriptorWriteData.desc = resDesc;
            descriptorWriteData.element = 0;
            descriptorWriteData.resource = constantBufferRef;

            descriptorSet.update(std::span{ &descriptorWriteData, 1 });

            commandListRecorder.acquireResourceForGraphics(
              gfx::PipelineStageFlagBits{ gfx::PipelineStageFlags::TOP_OF_PIPE_BIT },
              gfx::PipelineStageFlagBits{ gfx::PipelineStageFlags::VERTEX_SHADER_BIT,
                                          gfx::PipelineStageFlags::FRAGMENT_SHADER_BIT },
              gfx::AccessFlagBits{},
              gfx::AccessFlagBits{ gfx::AccessFlags::UNIFORM_READ_BIT },
              constantBufferRef);

            auto bindingSchemaRef = material.pipeline().as<gfx::Pipeline>().bindingSchema();
            commandListRecorder.bindDescriptorSet(gfx::PipelineBindPoint::GRAPHICS, bindingSchemaRef, descriptorSetRef);

            commandListRecorder.draw(36, 1, 0, 0);

            commandListRecorder.releaseResourceToTransfer(
              gfx::PipelineStageFlagBits{ gfx::PipelineStageFlags::VERTEX_SHADER_BIT,
                                          gfx::PipelineStageFlags::FRAGMENT_SHADER_BIT },
              gfx::PipelineStageFlagBits{ gfx::PipelineStageFlags::BOTTOM_OF_PIPE_BIT },
              gfx::AccessFlagBits{ gfx::AccessFlags::UNIFORM_READ_BIT },
              gfx::AccessFlagBits{},
              constantBufferRef);

            commandListRecorder.endRenderPass();

            commandListRecorder.end();

            commandListRecorder.finish();

            batchRecorder.finish();
        }

        renderSubmissionRecorder.finish().get();
    }

    Renderer* renderer_;
};

void Renderer::render()
{
    transferTaskFuture_ = multithreading::Executor::threadExecutor().taskManager().submitTask(
      PassConstantsTransferJob{ this }, multithreading::Purpose::Transfer);

    multithreading::Executor::threadExecutor()
      .taskManager()
      .submitTask(PassRenderJob{ this }, multithreading::Purpose::Render)
      .get();
}
}
