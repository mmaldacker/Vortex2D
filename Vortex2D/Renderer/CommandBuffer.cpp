//
//  CommandBuffer.cpp
//  Vortex2D
//

#include "CommandBuffer.h"

#include <Vortex2D/Renderer/Device.h>
#include <Vortex2D/Renderer/Drawable.h>
#include <Vortex2D/Renderer/RenderTarget.h>

namespace Vortex
{
namespace Renderer
{
namespace
{
const uint32_t zero = 0;

}

CommandBuffer::CommandBuffer(const Device& device, bool synchronise)
    : mDevice(device)
    , mSynchronise(synchronise)
    , mRecorded(false)
    , mCommandBuffer(device.CreateCommandBuffer())
    , mFence(device.Handle().createFenceUnique({vk::FenceCreateFlagBits::eSignaled}))
{
}

CommandBuffer::~CommandBuffer()
{
  if (mCommandBuffer != vk::CommandBuffer(nullptr))
  {
    Wait().Reset();
    mDevice.FreeCommandBuffer(mCommandBuffer);
  }
}

CommandBuffer::CommandBuffer(CommandBuffer&& other)
    : mDevice(other.mDevice)
    , mSynchronise(other.mSynchronise)
    , mRecorded(other.mRecorded)
    , mCommandBuffer(other.mCommandBuffer)
    , mFence(std::move(other.mFence))
{
  other.mCommandBuffer = nullptr;
  other.mRecorded = false;
}

CommandBuffer& CommandBuffer::operator=(CommandBuffer&& other)
{
  assert(mDevice.Handle() == other.mDevice.Handle());
  mSynchronise = other.mSynchronise;
  mRecorded = other.mRecorded;
  mCommandBuffer = other.mCommandBuffer;
  mFence = std::move(other.mFence);

  other.mCommandBuffer = nullptr;
  other.mRecorded = false;

  return *this;
}

CommandBuffer& CommandBuffer::Record(CommandBuffer::CommandFn commandFn)
{
  Wait();

  auto bufferBegin =
      vk::CommandBufferBeginInfo().setFlags(vk::CommandBufferUsageFlagBits::eSimultaneousUse);

  mCommandBuffer.begin(bufferBegin);
  commandFn(mCommandBuffer);
  mCommandBuffer.end();
  mRecorded = true;

  return *this;
}

CommandBuffer& CommandBuffer::Record(const RenderTarget& renderTarget,
                                     vk::Framebuffer framebuffer,
                                     CommandFn commandFn)
{
  Wait();

  auto bufferBegin =
      vk::CommandBufferBeginInfo().setFlags(vk::CommandBufferUsageFlagBits::eSimultaneousUse);

  mCommandBuffer.begin(bufferBegin);

  auto renderPassBegin = vk::RenderPassBeginInfo()
                             .setFramebuffer(framebuffer)
                             .setRenderPass(*renderTarget.RenderPass)
                             .setRenderArea({{0, 0}, {renderTarget.Width, renderTarget.Height}});

  mCommandBuffer.beginRenderPass(renderPassBegin, vk::SubpassContents::eInline);

  commandFn(mCommandBuffer);

  mCommandBuffer.endRenderPass();
  mCommandBuffer.end();
  mRecorded = true;

  return *this;
}

CommandBuffer& CommandBuffer::Wait()
{
  if (mSynchronise)
  {
    mDevice.Handle().waitForFences({*mFence}, true, UINT64_MAX);
  }

  return *this;
}

CommandBuffer& CommandBuffer::Reset()
{
  if (mSynchronise)
  {
    mDevice.Handle().resetFences({*mFence});
  }

  return *this;
}

CommandBuffer& CommandBuffer::Submit(const std::initializer_list<vk::Semaphore>& waitSemaphores,
                                     const std::initializer_list<vk::Semaphore>& signalSemaphores)
{
  if (!mRecorded)
    throw std::runtime_error("Submitting a command that wasn't recorded");

  Reset();

  std::vector<vk::PipelineStageFlags> waitStages(waitSemaphores.size(),
                                                 vk::PipelineStageFlagBits::eAllCommands);

  auto submitInfo = vk::SubmitInfo()
                        .setCommandBufferCount(1)
                        .setPCommandBuffers(&mCommandBuffer)
                        .setWaitSemaphoreCount(static_cast<uint32_t>(waitSemaphores.size()))
                        .setPWaitSemaphores(waitSemaphores.begin())
                        .setSignalSemaphoreCount(static_cast<uint32_t>(signalSemaphores.size()))
                        .setPSignalSemaphores(signalSemaphores.begin())
                        .setPWaitDstStageMask(waitStages.data());

  if (mSynchronise)
  {
    mDevice.Queue().submit({submitInfo}, *mFence);
  }
  else
  {
    mDevice.Queue().submit({submitInfo}, nullptr);
  }

  return *this;
}

CommandBuffer::operator bool() const
{
  return mRecorded;
}

RenderCommand::RenderCommand(RenderCommand&& other)
    : mRenderTarget(other.mRenderTarget)
    , mCmds(std::move(other.mCmds))
    , mIndex(other.mIndex)
    , mDrawables(std::move(other.mDrawables))
    , mView(other.mView)
{
  other.mRenderTarget = nullptr;
  other.mIndex = nullptr;
}

RenderCommand::RenderCommand() : mRenderTarget(nullptr), mIndex(&zero) {}

RenderCommand::~RenderCommand()
{
  for (auto& cmd : mCmds)
  {
    cmd.Wait();
  }
}

RenderCommand& RenderCommand::operator=(RenderCommand&& other)
{
  mRenderTarget = other.mRenderTarget;
  mCmds = std::move(other.mCmds);
  mIndex = other.mIndex;
  mDrawables = std::move(other.mDrawables);

  other.mRenderTarget = nullptr;
  other.mIndex = nullptr;

  return *this;
}

RenderCommand::RenderCommand(const Device& device,
                             RenderTarget& renderTarget,
                             const RenderState& renderState,
                             const vk::UniqueFramebuffer& frameBuffer,
                             RenderTarget::DrawableList drawables)
    : mRenderTarget(&renderTarget), mIndex(&zero), mDrawables(drawables), mView(1.0f)
{
  for (auto& drawable : drawables)
  {
    drawable.get().Initialize(renderState);
  }

  CommandBuffer cmd(device, true);
  cmd.Record(renderTarget, *frameBuffer, [&](vk::CommandBuffer commandBuffer) {
    for (auto& drawable : drawables)
    {
      drawable.get().Draw(commandBuffer, renderState);
    }
  });

  mCmds.emplace_back(std::move(cmd));
}

RenderCommand::RenderCommand(const Device& device,
                             RenderTarget& renderTarget,
                             const RenderState& renderState,
                             const std::vector<vk::UniqueFramebuffer>& frameBuffers,
                             const uint32_t& index,
                             RenderTarget::DrawableList drawables)
    : mRenderTarget(&renderTarget), mIndex(&index), mDrawables(drawables), mView(1.0f)
{
  for (auto& drawable : drawables)
  {
    drawable.get().Initialize(renderState);
  }

  for (auto& frameBuffer : frameBuffers)
  {
    CommandBuffer cmd(device, true);
    cmd.Record(renderTarget, *frameBuffer, [&](vk::CommandBuffer commandBuffer) {
      for (auto& drawable : drawables)
      {
        drawable.get().Draw(commandBuffer, renderState);
      }
    });

    mCmds.emplace_back(std::move(cmd));
  }
}

RenderCommand& RenderCommand::Submit(const glm::mat4& view)
{
  mView = view;

  if (mRenderTarget)
  {
    mRenderTarget->Submit(*this);
  }

  return *this;
}

void RenderCommand::Wait()
{
  assert(mIndex);
  assert(*mIndex < mCmds.size());
  mCmds[*mIndex].Wait();
}

void RenderCommand::Render(const std::initializer_list<vk::Semaphore>& waitSemaphores,
                           const std::initializer_list<vk::Semaphore>& signalSemaphores)
{
  assert(mIndex);
  if (mCmds.empty())
    return;
  if (*mIndex >= mCmds.size())
    throw std::runtime_error("invalid index");

  for (auto& drawable : mDrawables)
  {
    assert(mRenderTarget);
    drawable.get().Update(mRenderTarget->Orth, mRenderTarget->View * mView);
  }

  mCmds[*mIndex].Wait();
  mCmds[*mIndex].Submit(waitSemaphores, signalSemaphores);
}

RenderCommand::operator bool() const
{
  return !mCmds.empty();
}

}  // namespace Renderer
}  // namespace Vortex2D
