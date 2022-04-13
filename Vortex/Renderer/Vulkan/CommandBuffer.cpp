//
//  CommandBuffer.cpp
//  Vortex
//

#include <Vortex/Renderer/CommandBuffer.h>

#include <Vortex/Renderer/Drawable.h>
#include <Vortex/Renderer/RenderTarget.h>
#include <Vortex/Renderer/Vulkan/Vulkan.h>

#include "Device.h"

namespace Vortex
{
namespace Renderer
{
namespace
{
const uint32_t zero = 0;
}

struct CommandEncoder::Impl
{
  Impl(Device& device)
      : mDevice(static_cast<VulkanDevice&>(device)), mCommandBuffer(mDevice.CreateCommandBuffer())
  {
  }

  void Begin()
  {
    auto bufferBegin =
        vk::CommandBufferBeginInfo().setFlags(vk::CommandBufferUsageFlagBits::eSimultaneousUse);

    mCommandBuffer->begin(bufferBegin);
  }

  void BeginRenderPass(const RenderTarget& renderTarget, Handle::Framebuffer framebuffer)
  {
    auto renderPassBegin =
        vk::RenderPassBeginInfo()
            .setFramebuffer(reinterpret_cast<VkFramebuffer>(framebuffer))
            .setRenderPass(reinterpret_cast<VkRenderPass>(renderTarget.GetRenderPass()))
            .setRenderArea({{0, 0}, {renderTarget.GetWidth(), renderTarget.GetHeight()}});

    mCommandBuffer->beginRenderPass(renderPassBegin, vk::SubpassContents::eInline);
  }

  void EndRenderPass() { mCommandBuffer->endRenderPass(); }

  void End() { mCommandBuffer->end(); }

  void SetPipeline(PipelineBindPoint pipelineBindPoint, vk::Pipeline pipeline)
  {
    mCommandBuffer->bindPipeline(ConvertPipelineBindPoint(pipelineBindPoint), pipeline);
  }

  void SetBindGroup(PipelineBindPoint pipelineBindPoint,
                    vk::PipelineLayout layout,
                    BindGroup& bindGroup)
  {
    vk::DescriptorSet descriptorSet = reinterpret_cast<VkDescriptorSet>(bindGroup.Handle());
    mCommandBuffer->bindDescriptorSets(
        ConvertPipelineBindPoint(pipelineBindPoint), layout, 0, {descriptorSet}, {});
  }

  void SetVertexBuffer(const GenericBuffer& buffer)
  {
    mCommandBuffer->bindVertexBuffers(0, {Handle::ConvertBuffer(buffer.Handle())}, {0ul});
  }

  void SetIndexBuffer(const GenericBuffer& buffer)
  {
    mCommandBuffer->bindIndexBuffer(
        Handle::ConvertBuffer(buffer.Handle()), 0, vk::IndexType::eUint32);
  }

  void PushConstants(vk::PipelineLayout layout,
                     ShaderStage stageFlags,
                     uint32_t offset,
                     uint32_t size,
                     const void* pValues)
  {
    mCommandBuffer->pushConstants(layout, ConvertShaderStage(stageFlags), offset, size, pValues);
  }

  void Draw(std::uint32_t vertexCount) { mCommandBuffer->draw(vertexCount, 1, 0, 0); }

  void DrawIndexedIndirect(const GenericBuffer& buffer)
  {
    mCommandBuffer->drawIndexedIndirect(Handle::ConvertBuffer(buffer.Handle()), 0, 1, 0);
  }

  void Dispatch(std::uint32_t x, std::uint32_t y, std::uint32_t z)
  {
    mCommandBuffer->dispatch(x, y, z);
  }

  void DispatchIndirect(GenericBuffer& buffer)
  {
    mCommandBuffer->dispatchIndirect(Handle::ConvertBuffer(buffer.Handle()), 0);
  }

  void Clear(const glm::ivec2& pos, const glm::uvec2& size, const glm::vec4& colour)
  {
    auto clearValue =
        vk::ClearValue().setColor(std::array<float, 4>{{colour.r, colour.g, colour.b, colour.a}});

    auto clearAttachement = vk::ClearAttachment()
                                .setAspectMask(vk::ImageAspectFlagBits::eColor)
                                .setClearValue(clearValue);

    auto clearRect = vk::ClearRect().setRect({{pos.x, pos.y}, {size.x, size.y}}).setLayerCount(1);

    mCommandBuffer->clearAttachments({clearAttachement}, {clearRect});
  }

  void DebugMarkerBegin(const char* name, const glm::vec4& color)
  {
    mCommandBuffer->debugMarkerBeginEXT({name, {{color.r, color.g, color.b, color.a}}},
                                        mDevice.Loader());
  }

  void DebugMarkerEnd() { mCommandBuffer->debugMarkerEndEXT(mDevice.Loader()); }

  vk::CommandBuffer Handle() { return *mCommandBuffer; }

  VulkanDevice& mDevice;
  vk::UniqueCommandBuffer mCommandBuffer;
};

CommandEncoder::CommandEncoder(Device& device) : mImpl(std::make_unique<Impl>(device)) {}

CommandEncoder::CommandEncoder(CommandEncoder&& other) : mImpl(std::move(other.mImpl)) {}

CommandEncoder::~CommandEncoder() {}

void CommandEncoder::Begin()
{
  mImpl->Begin();
}

void CommandEncoder::BeginRenderPass(const RenderTarget& renderTarget,
                                     Handle::Framebuffer framebuffer)
{
  mImpl->BeginRenderPass(renderTarget, framebuffer);
}

void CommandEncoder::EndRenderPass()
{
  mImpl->EndRenderPass();
}

void CommandEncoder::End()
{
  mImpl->End();
}

void CommandEncoder::SetPipeline(PipelineBindPoint pipelineBindPoint, Handle::Pipeline pipeline)
{
  mImpl->SetPipeline(pipelineBindPoint, reinterpret_cast<VkPipeline>(pipeline));
}

void CommandEncoder::SetBindGroup(PipelineBindPoint pipelineBindPoint,
                                  Handle::PipelineLayout layout,
                                  BindGroup& bindGroup)
{
  mImpl->SetBindGroup(pipelineBindPoint, reinterpret_cast<VkPipelineLayout>(layout), bindGroup);
}

void CommandEncoder::SetVertexBuffer(const GenericBuffer& buffer)
{
  mImpl->SetVertexBuffer(buffer);
}

void CommandEncoder::SetIndexBuffer(const GenericBuffer& buffer)
{
  mImpl->SetIndexBuffer(buffer);
}

void CommandEncoder::PushConstants(Handle::PipelineLayout layout,
                                   ShaderStage stageFlags,
                                   uint32_t offset,
                                   uint32_t size,
                                   const void* pValues)
{
  mImpl->PushConstants(
      reinterpret_cast<VkPipelineLayout>(layout), stageFlags, offset, size, pValues);
}

void CommandEncoder::Draw(std::uint32_t vertexCount)
{
  mImpl->Draw(vertexCount);
}

void CommandEncoder::DrawIndexedIndirect(const GenericBuffer& buffer)
{
  mImpl->DrawIndexedIndirect(buffer);
}

void CommandEncoder::Dispatch(std::uint32_t x, std::uint32_t y, std::uint32_t z)
{
  mImpl->Dispatch(x, y, z);
}

void CommandEncoder::DispatchIndirect(GenericBuffer& buffer)
{
  mImpl->DispatchIndirect(buffer);
}

void CommandEncoder::Clear(const glm::ivec2& pos, const glm::uvec2& size, const glm::vec4& colour)
{
  mImpl->Clear(pos, size, colour);
}

void CommandEncoder::DebugMarkerBegin(const char* name, const glm::vec4& color)
{
  mImpl->DebugMarkerBegin(name, color);
}

void CommandEncoder::DebugMarkerEnd()
{
  mImpl->DebugMarkerEnd();
}

Handle::CommandBuffer CommandEncoder::Handle()
{
  VkCommandBuffer commandBuffer = mImpl->Handle();
  return reinterpret_cast<Handle::CommandBuffer>(commandBuffer);
}

struct CommandBuffer::Impl
{
  Impl(Device& device, bool synchronise)
      : mDevice(static_cast<VulkanDevice&>(device))
      , mSynchronise(synchronise)
      , mRecorded(false)
      , mCommandEncoder(device)
      , mFence(mDevice.Handle().createFenceUnique({vk::FenceCreateFlagBits::eSignaled}))
  {
  }

  void Record(CommandBuffer::CommandFn commandFn)
  {
    Wait();

    mCommandEncoder.Begin();
    commandFn(mCommandEncoder);
    mCommandEncoder.End();
    mRecorded = true;
  }

  void Record(const RenderTarget& renderTarget,
              Handle::Framebuffer framebuffer,
              CommandFn commandFn)
  {
    Wait();

    mCommandEncoder.Begin();
    mCommandEncoder.BeginRenderPass(renderTarget, framebuffer);

    commandFn(mCommandEncoder);

    mCommandEncoder.EndRenderPass();
    mCommandEncoder.End();
    mRecorded = true;
  }

  void Wait()
  {
    if (mSynchronise)
    {
      mDevice.Handle().waitForFences({*mFence}, true, UINT64_MAX);
    }
  }

  void Reset()
  {
    if (mSynchronise)
    {
      mDevice.Handle().resetFences({*mFence});
    }
  }

  void Submit(const std::initializer_list<Handle::Semaphore>& waitSemaphores,
              const std::initializer_list<Handle::Semaphore>& signalSemaphores)
  {
    if (!mRecorded)
      throw std::runtime_error("Submitting a command that wasn't recorded");

    Reset();

    std::vector<vk::PipelineStageFlags> waitStages(waitSemaphores.size(),
                                                   vk::PipelineStageFlagBits::eAllCommands);

    vk::CommandBuffer commandBuffers[] = {
        reinterpret_cast<VkCommandBuffer>(mCommandEncoder.Handle())};

    std::vector<vk::Semaphore> waitS;
    for (auto& semaphore : waitSemaphores)
    {
      waitS.push_back(Handle::ConvertSemaphore(semaphore));
    }

    std::vector<vk::Semaphore> signalS;
    for (auto& semaphore : signalSemaphores)
    {
      signalS.push_back(Handle::ConvertSemaphore(semaphore));
    }

    auto submitInfo = vk::SubmitInfo()
                          .setCommandBufferCount(1)
                          .setPCommandBuffers(commandBuffers)
                          .setWaitSemaphoreCount(static_cast<uint32_t>(waitSemaphores.size()))
                          .setPWaitSemaphores(waitS.data())
                          .setSignalSemaphoreCount(static_cast<uint32_t>(signalSemaphores.size()))
                          .setPSignalSemaphores(signalS.data())
                          .setPWaitDstStageMask(waitStages.data());

    if (mSynchronise)
    {
      mDevice.Queue().submit({submitInfo}, *mFence);
    }
    else
    {
      mDevice.Queue().submit({submitInfo}, nullptr);
    }
  }

  bool IsValid() const { return mRecorded; }

  VulkanDevice& mDevice;
  bool mSynchronise;
  bool mRecorded;
  CommandEncoder mCommandEncoder;
  vk::UniqueFence mFence;
};

CommandBuffer::CommandBuffer(Device& device, bool synchronise)
    : mImpl(std::make_unique<Impl>(device, synchronise))
{
}

CommandBuffer::CommandBuffer(CommandBuffer&& other) : mImpl(std::move(other.mImpl)) {}

CommandBuffer::~CommandBuffer() {}

CommandBuffer& CommandBuffer::Record(CommandFn commandFn)
{
  mImpl->Record(commandFn);
  return *this;
}

CommandBuffer& CommandBuffer::Record(const RenderTarget& renderTarget,
                                     Handle::Framebuffer framebuffer,
                                     CommandFn commandFn)
{
  mImpl->Record(renderTarget, framebuffer, commandFn);
  return *this;
}

CommandBuffer& CommandBuffer::Wait()
{
  mImpl->Wait();
  return *this;
}

CommandBuffer& CommandBuffer::Reset()
{
  mImpl->Reset();
  return *this;
}

CommandBuffer& CommandBuffer::Submit(
    const std::initializer_list<Handle::Semaphore>& waitSemaphores,
    const std::initializer_list<Handle::Semaphore>& signalSemaphores)
{
  mImpl->Submit(waitSemaphores, signalSemaphores);
  return *this;
}

CommandBuffer::operator bool() const
{
  return mImpl->IsValid();
}

struct RenderCommand::Impl
{
  Impl(RenderCommand& self) : mSelf(&self), mRenderTarget(nullptr), mIndex(&zero) {}

  Impl(RenderCommand& self,
       Device& device,
       RenderTarget& renderTarget,
       const RenderState& renderState,
       const Handle::Framebuffer& frameBuffer,
       RenderTarget::DrawableList drawables)
      : mSelf(&self)
      , mRenderTarget(&renderTarget)
      , mIndex(&zero)
      , mDrawables(drawables)
      , mView(1.0f)
  {
    for (auto& drawable : drawables)
    {
      drawable->Initialize(renderState);
    }

    CommandBuffer cmd(device, true);
    cmd.Record(renderTarget,
               frameBuffer,
               [&](CommandEncoder& command)
               {
                 for (auto& drawable : drawables)
                 {
                   drawable->Draw(command, renderState);
                 }
               });

    mCmds.emplace_back(std::move(cmd));
  }

  Impl(RenderCommand& self,
       Device& device,
       RenderTarget& renderTarget,
       const RenderState& renderState,
       const std::vector<Handle::Framebuffer>& frameBuffers,
       const uint32_t& index,
       RenderTarget::DrawableList drawables)
      : mSelf(&self)
      , mRenderTarget(&renderTarget)
      , mIndex(&index)
      , mDrawables(drawables)
      , mView(1.0f)
  {
    for (auto& drawable : drawables)
    {
      drawable->Initialize(renderState);
    }

    for (auto& frameBuffer : frameBuffers)
    {
      CommandBuffer cmd(device, true);
      cmd.Record(renderTarget,
                 frameBuffer,
                 [&](CommandEncoder& command)
                 {
                   for (auto& drawable : drawables)
                   {
                     drawable->Draw(command, renderState);
                   }
                 });

      mCmds.emplace_back(std::move(cmd));
    }
  }

  ~Impl()
  {
    for (auto& cmd : mCmds)
    {
      cmd.Wait();
    }
  }

  void Submit(const glm::mat4& view)
  {
    mView = view;

    if (mRenderTarget)
    {
      mRenderTarget->Submit(*mSelf);
    }
  }

  void Wait()
  {
    assert(mIndex);
    assert(*mIndex < mCmds.size());
    mCmds[*mIndex].Wait();
  }

  void Render(const std::initializer_list<Handle::Semaphore>& waitSemaphores,
              const std::initializer_list<Handle::Semaphore>& signalSemaphores)
  {
    assert(mIndex);
    if (mCmds.empty())
      return;
    if (*mIndex >= mCmds.size())
      throw std::runtime_error("invalid index");

    for (auto& drawable : mDrawables)
    {
      assert(mRenderTarget);
      drawable->Update(mRenderTarget->GetOrth(), mRenderTarget->GetView() * mView);
    }

    mCmds[*mIndex].Wait();
    mCmds[*mIndex].Submit(waitSemaphores, signalSemaphores);
  }

  bool IsValid() const { return !mCmds.empty(); }

  RenderCommand* mSelf;
  RenderTarget* mRenderTarget;
  std::vector<CommandBuffer> mCmds;
  const uint32_t* mIndex;
  RenderTarget::DrawableList mDrawables;
  glm::mat4 mView;
};

RenderCommand::RenderCommand() : mImpl(std::make_unique<Impl>(*this)) {}

RenderCommand::~RenderCommand() {}

RenderCommand::RenderCommand(RenderCommand&& other) : mImpl(std::move(other.mImpl))
{
  mImpl->mSelf = this;
}

RenderCommand::RenderCommand(Device& device,
                             RenderTarget& renderTarget,
                             const RenderState& renderState,
                             const Handle::Framebuffer& frameBuffer,
                             RenderTarget::DrawableList drawables)
    : mImpl(std::make_unique<Impl>(*this,
                                   device,
                                   renderTarget,
                                   renderState,
                                   std::move(frameBuffer),
                                   drawables))
{
}

RenderCommand::RenderCommand(Device& device,
                             RenderTarget& renderTarget,
                             const RenderState& renderState,
                             const std::vector<Handle::Framebuffer>& frameBuffers,
                             const uint32_t& index,
                             RenderTarget::DrawableList drawables)
    : mImpl(std::make_unique<Impl>(*this,
                                   device,
                                   renderTarget,
                                   renderState,
                                   std::move(frameBuffers),
                                   index,
                                   drawables))
{
}

RenderCommand& RenderCommand::operator=(RenderCommand&& other)
{
  mImpl = std::move(other.mImpl);
  mImpl->mSelf = this;
  return *this;
}

RenderCommand& RenderCommand::Submit(const glm::mat4& view)
{
  mImpl->Submit(view);
  return *this;
}

void RenderCommand::Wait()
{
  mImpl->Wait();
}

void RenderCommand::Render(const std::initializer_list<Handle::Semaphore>& waitSemaphores,
                           const std::initializer_list<Handle::Semaphore>& signalSemaphores)
{
  mImpl->Render(waitSemaphores, signalSemaphores);
}

RenderCommand::operator bool() const
{
  return mImpl->IsValid();
}

}  // namespace Renderer
}  // namespace Vortex
