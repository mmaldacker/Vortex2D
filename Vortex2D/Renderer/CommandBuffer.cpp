//
//  CommandBuffer.cpp
//  Vortex2D
//

#include "CommandBuffer.h"

namespace Vortex2D { namespace Renderer {

CommandBuffer::CommandBuffer(const Device& device)
    : mDevice(device)
    , mCommandBuffer(device.CreateCommandBuffers(1).at(0))
    , mFence(device.Handle().createFenceUnique({vk::FenceCreateFlagBits::eSignaled}))
{

}

CommandBuffer::~CommandBuffer()
{
    mDevice.FreeCommandBuffers({mCommandBuffer});
}

void CommandBuffer::Record(CommandBuffer::CommandFn commandFn)
{
    auto bufferBegin = vk::CommandBufferBeginInfo()
            .setFlags(vk::CommandBufferUsageFlagBits::eSimultaneousUse);

    mCommandBuffer.begin(bufferBegin);
    commandFn(mCommandBuffer);
    mCommandBuffer.end();
}

void CommandBuffer::Wait()
{
    mDevice.Handle().waitForFences({*mFence}, true, UINT64_MAX);
    mDevice.Handle().resetFences({*mFence});
}

void CommandBuffer::Submit()
{
    auto submitInfo = vk::SubmitInfo()
            .setCommandBufferCount(1)
            .setPCommandBuffers(&mCommandBuffer);

    mDevice.Queue().submit({submitInfo}, *mFence);
}


}}
