//
//  Buffer.cpp
//  Vortex2D
//

#include "Buffer.h"

#include <Vortex2D/Renderer/CommandBuffer.h>
#include <Vortex2D/Renderer/Texture.h>

namespace Vortex2D { namespace Renderer {

void BufferBarrier(vk::Buffer buffer, vk::CommandBuffer commandBuffer, vk::AccessFlags oldAccess, vk::AccessFlags newAccess)
{
    auto bufferMemoryBarriers = vk::BufferMemoryBarrier()
            .setSrcQueueFamilyIndex(VK_QUEUE_FAMILY_IGNORED)
            .setDstQueueFamilyIndex(VK_QUEUE_FAMILY_IGNORED)
            .setBuffer(buffer)
            .setSize(VK_WHOLE_SIZE)
            .setSrcAccessMask(oldAccess)
            .setDstAccessMask(newAccess);

    commandBuffer.pipelineBarrier(vk::PipelineStageFlagBits::eAllCommands,
                                  vk::PipelineStageFlagBits::eAllCommands,
                                  {},
                                  nullptr,
                                  bufferMemoryBarriers,
                                  nullptr);
}

GenericBuffer::GenericBuffer(const Device& device, vk::BufferUsageFlags usageFlags, bool host, vk::DeviceSize deviceSize)
    : mDevice(device.Handle())
    , mHost(host)
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

        // TODO should we always clear in constructor
    ExecuteCommand(device, [&](vk::CommandBuffer commandBuffer)
    {
        Clear(commandBuffer);
    });
}

vk::Buffer GenericBuffer::Handle() const
{
    return *mBuffer;
}

vk::DeviceSize GenericBuffer::Size() const
{
    return mSize;
}

void GenericBuffer::CopyFrom(vk::CommandBuffer commandBuffer, GenericBuffer& srcBuffer)
{
    if (mSize != srcBuffer.mSize)
    {
        throw std::runtime_error("Cannot copy buffers of different sizes");
    }

    // TODO improve barriers
    srcBuffer.Barrier(commandBuffer, vk::AccessFlagBits::eShaderWrite, vk::AccessFlagBits::eTransferRead);
    Barrier(commandBuffer, vk::AccessFlagBits::eShaderRead, vk::AccessFlagBits::eTransferWrite);

    auto region = vk::BufferCopy()
            .setSize(mSize);

    commandBuffer.copyBuffer(srcBuffer.Handle(), *mBuffer, region);

    Barrier(commandBuffer, vk::AccessFlagBits::eTransferWrite, vk::AccessFlagBits::eShaderRead);
    srcBuffer.Barrier(commandBuffer, vk::AccessFlagBits::eTransferRead, vk::AccessFlagBits::eShaderRead);
}

void GenericBuffer::CopyFrom(vk::CommandBuffer commandBuffer, Texture& srcTexture)
{
    // TODO check if it can be copied

    srcTexture.Barrier(commandBuffer,
                       vk::ImageLayout::eGeneral,
                       vk::AccessFlagBits::eShaderWrite | vk::AccessFlagBits::eColorAttachmentWrite,
                       vk::ImageLayout::eTransferSrcOptimal,
                       vk::AccessFlagBits::eTransferRead);

    auto info = vk::BufferImageCopy()
            .setImageSubresource({vk::ImageAspectFlagBits::eColor, 0, 0 ,1})
            .setImageExtent({srcTexture.GetWidth(), srcTexture.GetHeight(), 1});

    commandBuffer.copyImageToBuffer(*srcTexture.mImage,
                                    vk::ImageLayout::eTransferSrcOptimal,
                                    *mBuffer,
                                    info);

    srcTexture.Barrier(commandBuffer,
                       vk::ImageLayout::eTransferSrcOptimal,
                       vk::AccessFlagBits::eTransferRead,
                       vk::ImageLayout::eGeneral,
                       vk::AccessFlagBits::eShaderRead | vk::AccessFlagBits::eColorAttachmentRead);

    Barrier(commandBuffer, vk::AccessFlagBits::eTransferWrite, vk::AccessFlagBits::eShaderRead);
}

void GenericBuffer::Barrier(vk::CommandBuffer commandBuffer, vk::AccessFlags oldAccess, vk::AccessFlags newAccess)
{
    BufferBarrier(*mBuffer, commandBuffer, oldAccess, newAccess);
}

void GenericBuffer::Clear(vk::CommandBuffer commandBuffer)
{
    Barrier(commandBuffer, vk::AccessFlagBits::eShaderRead, vk::AccessFlagBits::eTransferWrite);
    commandBuffer.fillBuffer(*mBuffer, 0, mSize, 0);
    Barrier(commandBuffer, vk::AccessFlagBits::eTransferWrite, vk::AccessFlagBits::eShaderRead);
}

void GenericBuffer::CopyFrom(const void* data)
{
    if (!mHost) throw std::runtime_error("Not local buffer");
    void* mapped = mDevice.mapMemory(*mMemory, 0, mSize, vk::MemoryMapFlagBits());
    std::memcpy(mapped, data, mSize);
    mDevice.unmapMemory(*mMemory);
}

void GenericBuffer::CopyTo(void* data)
{
    if (!mHost) throw std::runtime_error("Not local buffer");
    void* mapped = mDevice.mapMemory(*mMemory, 0, mSize, vk::MemoryMapFlagBits());
    std::memcpy(data, mapped, mSize);
    mDevice.unmapMemory(*mMemory);
}

}}
