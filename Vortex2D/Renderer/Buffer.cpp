//
//  Buffer.cpp
//  Vortex2D
//

#include "Buffer.h"

namespace Vortex2D { namespace Renderer {

Buffer::Buffer(const Device& device, vk::BufferUsageFlags usageFlags, bool host, vk::DeviceSize deviceSize)
    : mDevice(device.Handle())
    , mSize(deviceSize)
{
    usageFlags |= vk::BufferUsageFlagBits::eTransferDst |
                  vk::BufferUsageFlagBits::eTransferSrc;

    auto bufferInfo = vk::BufferCreateInfo()
            .setSize(deviceSize)
            .setUsage(usageFlags)
            .setSharingMode(vk::SharingMode::eExclusive);

    mBuffer = device.Handle().createBufferUnique(bufferInfo);

    auto memoryRequirements = device.Handle().getBufferMemoryRequirements(*mBuffer);

    vk::MemoryPropertyFlags memoryFlags = host ? vk::MemoryPropertyFlagBits::eHostVisible |
                                                 vk::MemoryPropertyFlagBits::eHostCoherent
                                               : vk::MemoryPropertyFlagBits::eDeviceLocal;

    uint32_t memoryPropertyIndex =
            device.FindMemoryPropertiesIndex(memoryRequirements.memoryTypeBits, memoryFlags);

    auto memoryInfo = vk::MemoryAllocateInfo()
            .setAllocationSize(memoryRequirements.size)
            .setMemoryTypeIndex(memoryPropertyIndex);

    mMemory = device.Handle().allocateMemoryUnique(memoryInfo);
    device.Handle().bindBufferMemory(*mBuffer, *mMemory, 0);


}

void Buffer::Flush()
{
    auto mappedRange = vk::MappedMemoryRange()
            .setMemory(*mMemory)
            .setSize(mSize);

    mDevice.flushMappedMemoryRanges({mappedRange});
}

Buffer::operator vk::Buffer() const
{
    return *mBuffer;
}

vk::DeviceSize Buffer::Size() const
{
    return mSize;
}

void Buffer::CopyFrom(vk::CommandBuffer commandBuffer, Buffer& srcBuffer)
{
    if (mSize != srcBuffer.mSize)
    {
        return;
    }

    // TODO improve barriers
    srcBuffer.Barrier(commandBuffer, vk::AccessFlagBits::eShaderWrite, vk::AccessFlagBits::eTransferRead);
    Barrier(commandBuffer, vk::AccessFlagBits::eShaderRead, vk::AccessFlagBits::eTransferWrite);

    auto region = vk::BufferCopy()
            .setSize(mSize);

    commandBuffer.copyBuffer(srcBuffer, *mBuffer, region);
}

void Buffer::Barrier(vk::CommandBuffer commandBuffer, vk::AccessFlags oldAccess, vk::AccessFlags newAccess)
{
    auto bufferMemoryBarriers = vk::BufferMemoryBarrier()
            .setSrcQueueFamilyIndex(VK_QUEUE_FAMILY_IGNORED)
            .setDstQueueFamilyIndex(VK_QUEUE_FAMILY_IGNORED)
            .setBuffer(*mBuffer)
            .setSize(mSize)
            .setSrcAccessMask(oldAccess)
            .setDstAccessMask(newAccess);

    commandBuffer.pipelineBarrier(vk::PipelineStageFlagBits::eAllCommands,
                                  vk::PipelineStageFlagBits::eAllCommands,
                                  {},
                                  nullptr,
                                  bufferMemoryBarriers,
                                  nullptr);
}

void Buffer::Clear(vk::CommandBuffer commandBuffer)
{

    Barrier(commandBuffer, vk::AccessFlagBits::eShaderRead, vk::AccessFlagBits::eTransferWrite);
    commandBuffer.fillBuffer(*mBuffer, 0, mSize, 0);
    Barrier(commandBuffer, vk::AccessFlagBits::eTransferWrite, vk::AccessFlagBits::eShaderRead);
}

}}
