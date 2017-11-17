//
//  CommandBuffer.cpp
//  Vortex2D
//

#include "CommandBuffer.h"

#include <Vortex2D/Renderer/RenderTarget.h>

namespace Vortex2D { namespace Renderer {

CommandBuffer::CommandBuffer(const Device& device, bool synchronise)
    : mDevice(device)
    , mSynchronise(synchronise)
    , mCommandBuffer(device.CreateCommandBuffers(1).at(0))
    , mFence(device.Handle().createFenceUnique({vk::FenceCreateFlagBits::eSignaled}))
{

}

CommandBuffer::~CommandBuffer()
{
    if (mCommandBuffer != vk::CommandBuffer(nullptr))
    {
        mDevice.FreeCommandBuffers({mCommandBuffer});
    }
}

CommandBuffer::CommandBuffer(CommandBuffer&& other)
    : mDevice(other.mDevice)
    , mSynchronise(other.mSynchronise)
    , mCommandBuffer(other.mCommandBuffer)
    , mFence(std::move(other.mFence))
{
    other.mCommandBuffer = nullptr;
}

CommandBuffer& CommandBuffer::operator=(CommandBuffer&& other)
{
    assert(mDevice.Handle() == other.mDevice.Handle());
    mSynchronise = other.mSynchronise;
    mCommandBuffer = other.mCommandBuffer;
    mFence = std::move(other.mFence);

    other.mCommandBuffer = nullptr;

    return *this;
}

void CommandBuffer::Record(CommandBuffer::CommandFn commandFn)
{
    Wait();

    auto bufferBegin = vk::CommandBufferBeginInfo()
            .setFlags(vk::CommandBufferUsageFlagBits::eSimultaneousUse);

    mCommandBuffer.begin(bufferBegin);
    commandFn(mCommandBuffer);
    mCommandBuffer.end();
}

void CommandBuffer::Record(const RenderTarget& renderTarget, vk::Framebuffer framebuffer, CommandFn commandFn)
{
    Wait();

    auto bufferBegin = vk::CommandBufferBeginInfo()
            .setFlags(vk::CommandBufferUsageFlagBits::eSimultaneousUse);

    mCommandBuffer.begin(bufferBegin);

    auto renderPassBegin = vk::RenderPassBeginInfo()
            .setFramebuffer(framebuffer)
            .setRenderPass(*renderTarget.RenderPass)
            .setRenderArea({{0, 0}, {renderTarget.Width, renderTarget.Height}});

    mCommandBuffer.beginRenderPass(renderPassBegin, vk::SubpassContents::eInline);

    commandFn(mCommandBuffer);

    mCommandBuffer.endRenderPass();
    mCommandBuffer.end();
}

void CommandBuffer::Wait()
{
    if (mSynchronise)
    {
        mDevice.Handle().waitForFences({*mFence}, true, UINT64_MAX);
    }
}

void CommandBuffer::Reset()
{
    if (mSynchronise)
    {
        mDevice.Handle().resetFences({*mFence});
    }
}

void CommandBuffer::Submit(std::initializer_list<vk::Semaphore> waitSemaphores,
                           std::initializer_list<vk::Semaphore> signalSemaphores)
{
    Reset();

    std::vector<vk::PipelineStageFlags> waitStages(waitSemaphores.size(), vk::PipelineStageFlagBits::eAllCommands);

    auto submitInfo = vk::SubmitInfo()
            .setCommandBufferCount(1)
            .setPCommandBuffers(&mCommandBuffer)
            .setWaitSemaphoreCount(waitSemaphores.size())
            .setPWaitSemaphores(waitSemaphores.begin())
            .setSignalSemaphoreCount(signalSemaphores.size())
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
}

void ExecuteCommand(const Device& device, CommandBuffer::CommandFn commandFn)
{
    CommandBuffer cmd(device);
    cmd.Record(commandFn);
    cmd.Submit();
    cmd.Wait();
}


}}
