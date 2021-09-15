//
//  Buffer.cpp
//  Vortex
//

#include "Buffer.h"

#include <Vortex/Renderer/CommandBuffer.h>
#include <Vortex/Renderer/Device.h>
#include <Vortex/Renderer/Texture.h>

namespace Vortex
{
namespace Renderer
{
void BufferBarrier(vk::Buffer buffer,
                   vk::CommandBuffer commandBuffer,
                   vk::AccessFlags oldAccess,
                   vk::AccessFlags newAccess)
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

GenericBuffer::GenericBuffer(const Device& device,
                             vk::BufferUsageFlags usageFlags,
                             VmaMemoryUsage memoryUsage,
                             vk::DeviceSize deviceSize)
    : mDevice(device)
    , mSize(deviceSize)
    , mUsageFlags(usageFlags | vk::BufferUsageFlagBits::eTransferDst |
                  vk::BufferUsageFlagBits::eTransferSrc)
    , mMemoryUsage(memoryUsage)
{
  Create();
}

GenericBuffer::~GenericBuffer()
{
  if (mBuffer != VK_NULL_HANDLE)
  {
    vmaDestroyBuffer(mDevice.Allocator(), mBuffer, mAllocation);
  }
}

GenericBuffer::GenericBuffer(GenericBuffer&& other)
    : mDevice(other.mDevice)
    , mSize(other.mSize)
    , mBuffer(other.mBuffer)
    , mAllocation(other.mAllocation)
    , mAllocationInfo(other.mAllocationInfo)
{
  other.mBuffer = VK_NULL_HANDLE;
  other.mAllocation = VK_NULL_HANDLE;
  other.mSize = 0;
}

void GenericBuffer::Create()
{
  auto bufferInfo = vk::BufferCreateInfo()
                        .setSize(mSize)
                        .setUsage(mUsageFlags)
                        .setSharingMode(vk::SharingMode::eExclusive);

  VkBufferCreateInfo vkBufferInfo = bufferInfo;
  VmaAllocationCreateInfo allocInfo = {};
  allocInfo.usage = mMemoryUsage;
  if (vmaCreateBuffer(mDevice.Allocator(),
                      &vkBufferInfo,
                      &allocInfo,
                      &mBuffer,
                      &mAllocation,
                      &mAllocationInfo) != VK_SUCCESS)
  {
    throw std::runtime_error("Error creating buffer");
  }
}

vk::Buffer GenericBuffer::Handle() const
{
  return mBuffer;
}

vk::DeviceSize GenericBuffer::Size() const
{
  return mSize;
}

void GenericBuffer::Resize(vk::DeviceSize size)
{
  if (mBuffer != VK_NULL_HANDLE)
  {
    vmaDestroyBuffer(mDevice.Allocator(), mBuffer, mAllocation);
  }

  mSize = size;
  Create();
}

void GenericBuffer::CopyFrom(vk::CommandBuffer commandBuffer, GenericBuffer& srcBuffer)
{
  if (mSize != srcBuffer.mSize)
  {
    throw std::runtime_error("Cannot copy buffers of different sizes");
  }

  auto region = vk::BufferCopy().setSize(mSize);

  srcBuffer.Barrier(commandBuffer, {}, vk::AccessFlagBits::eMemoryRead);
  commandBuffer.copyBuffer(srcBuffer.Handle(), mBuffer, region);
  Barrier(commandBuffer, vk::AccessFlagBits::eMemoryWrite, vk::AccessFlagBits::eMemoryRead);
}

void GenericBuffer::CopyFrom(vk::CommandBuffer commandBuffer, Texture& srcTexture)
{
  auto textureSize =
      srcTexture.GetWidth() * srcTexture.GetHeight() * GetBytesPerPixel(srcTexture.GetFormat());
  if (textureSize != mSize)
  {
    throw std::runtime_error("Cannot copy texture of different sizes");
  }

  srcTexture.Barrier(commandBuffer,
                     vk::ImageLayout::eGeneral,
                     {},
                     vk::ImageLayout::eTransferSrcOptimal,
                     vk::AccessFlagBits::eMemoryRead);

  auto info = vk::BufferImageCopy()
                  .setImageSubresource({vk::ImageAspectFlagBits::eColor, 0, 0, 1})
                  .setImageExtent({srcTexture.GetWidth(), srcTexture.GetHeight(), 1});

  commandBuffer.copyImageToBuffer(
      srcTexture.mImage, vk::ImageLayout::eTransferSrcOptimal, mBuffer, info);

  srcTexture.Barrier(commandBuffer,
                     vk::ImageLayout::eTransferSrcOptimal,
                     vk::AccessFlagBits::eMemoryRead,
                     vk::ImageLayout::eGeneral,
                     vk::AccessFlagBits::eShaderRead | vk::AccessFlagBits::eColorAttachmentRead);

  Barrier(commandBuffer, vk::AccessFlagBits::eMemoryWrite, vk::AccessFlagBits::eMemoryRead);
}

void GenericBuffer::Barrier(vk::CommandBuffer commandBuffer,
                            vk::AccessFlags oldAccess,
                            vk::AccessFlags newAccess)
{
  BufferBarrier(mBuffer, commandBuffer, oldAccess, newAccess);
}

void GenericBuffer::Clear(vk::CommandBuffer commandBuffer)
{
  commandBuffer.fillBuffer(mBuffer, 0, mSize, 0);
  Barrier(commandBuffer, vk::AccessFlagBits::eMemoryWrite, vk::AccessFlagBits::eMemoryRead);
}

void GenericBuffer::CopyFrom(uint32_t offset, const void* data, uint32_t size)
{
  // TODO use always mapped functionality of VMA

  VkMemoryPropertyFlags memFlags;
  vmaGetMemoryTypeProperties(mDevice.Allocator(), mAllocationInfo.memoryType, &memFlags);
  if ((memFlags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT) == 0)
    throw std::runtime_error("Not visible buffer");

  void* pData;
  if (vmaMapMemory(mDevice.Allocator(), mAllocation, &pData) != VK_SUCCESS)
    throw std::runtime_error("Cannot map buffer");

  std::memcpy((uint8_t*)pData + offset, data, size);

  if ((memFlags & VK_MEMORY_PROPERTY_HOST_COHERENT_BIT) == 0)
  {
    vmaFlushAllocation(
        mDevice.Allocator(), mAllocation, mAllocationInfo.offset, mAllocationInfo.size);
  }

  vmaUnmapMemory(mDevice.Allocator(), mAllocation);
}

void GenericBuffer::CopyTo(uint32_t offset, void* data, uint32_t size)
{
  // TODO use always mapped functionality of VMA

  VkMemoryPropertyFlags memFlags;
  vmaGetMemoryTypeProperties(mDevice.Allocator(), mAllocationInfo.memoryType, &memFlags);
  if ((memFlags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT) == 0)
    throw std::runtime_error("Not visible buffer");

  void* pData;
  if (vmaMapMemory(mDevice.Allocator(), mAllocation, &pData) != VK_SUCCESS)
    throw std::runtime_error("Cannot map buffer");

  if ((memFlags & VK_MEMORY_PROPERTY_HOST_COHERENT_BIT) == 0)
  {
    vmaInvalidateAllocation(
        mDevice.Allocator(), mAllocation, mAllocationInfo.offset, mAllocationInfo.size);
  }

  std::memcpy(data, (uint8_t*)pData + offset, size);

  vmaUnmapMemory(mDevice.Allocator(), mAllocation);
}

}  // namespace Renderer
}  // namespace Vortex
