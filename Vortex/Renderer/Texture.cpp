//
//  Texture.cpp
//  Vortex
//

#include "Texture.h"

#include <Vortex/Renderer/CommandBuffer.h>
#include <Vortex/Renderer/Device.h>

namespace Vortex
{
namespace Renderer
{
vk::DeviceSize GetBytesPerPixel(vk::Format format)
{
  switch (format)
  {
    case vk::Format::eR8Uint:
    case vk::Format::eR8Sint:
      return 1;
    case vk::Format::eR32Sfloat:
    case vk::Format::eR32Sint:
    case vk::Format::eR8G8B8A8Unorm:
    case vk::Format::eB8G8R8A8Unorm:
      return 4;
    case vk::Format::eR32G32Sfloat:
      return 8;
    case vk::Format::eR32G32B32A32Sfloat:
      return 16;
    default:
      throw std::runtime_error("unsupported format");
  }
}

SamplerBuilder::SamplerBuilder()
{
  // TODO add sampler configuration
  mSamplerInfo = vk::SamplerCreateInfo().setMaxAnisotropy(1.0f);
}

SamplerBuilder& SamplerBuilder::AddressMode(vk::SamplerAddressMode mode)
{
  mSamplerInfo.setAddressModeU(mode).setAddressModeV(mode);
  return *this;
}

SamplerBuilder& SamplerBuilder::Filter(vk::Filter filter)
{
  mSamplerInfo.setMagFilter(filter).setMinFilter(filter);
  return *this;
}

vk::UniqueSampler SamplerBuilder::Create(vk::Device device)
{
  return device.createSamplerUnique(mSamplerInfo);
}

Texture::Texture(Device& device,
                 uint32_t width,
                 uint32_t height,
                 vk::Format format,
                 VmaMemoryUsage memoryUsage)
    : mDevice(device), mWidth(width), mHeight(height), mFormat(format)
{
  vk::ImageUsageFlags usageFlags =
      vk::ImageUsageFlagBits::eTransferSrc | vk::ImageUsageFlagBits::eTransferDst;

  vk::ImageLayout imageLayout = memoryUsage == VMA_MEMORY_USAGE_CPU_ONLY
                                    ? vk::ImageLayout::ePreinitialized
                                    : vk::ImageLayout::eUndefined;

  if (memoryUsage != VMA_MEMORY_USAGE_CPU_ONLY)
  {
    usageFlags |= vk::ImageUsageFlagBits::eSampled | vk::ImageUsageFlagBits::eStorage |
                  vk::ImageUsageFlagBits::eColorAttachment;
  }

  auto imageInfo =
      vk::ImageCreateInfo()
          .setImageType(vk::ImageType::e2D)
          .setExtent({width, height, 1})
          .setMipLevels(1)
          .setArrayLayers(1)
          .setFormat(format)
          .setTiling(memoryUsage == VMA_MEMORY_USAGE_CPU_ONLY ? vk::ImageTiling::eLinear
                                                              : vk::ImageTiling::eOptimal)
          .setInitialLayout(imageLayout)
          .setUsage(usageFlags)
          .setSharingMode(vk::SharingMode::eExclusive)
          .setSamples(vk::SampleCountFlagBits::e1);

  VkImageCreateInfo vkImageInfo = imageInfo;
  VmaAllocationCreateInfo allocInfo = {};
  allocInfo.usage = memoryUsage;
  if (vmaCreateImage(
          device.Allocator(), &vkImageInfo, &allocInfo, &mImage, &mAllocation, &mAllocationInfo) !=
      VK_SUCCESS)
  {
    throw std::runtime_error("Error creating texture");
  }

  if (memoryUsage != VMA_MEMORY_USAGE_CPU_ONLY)
  {
    auto imageViewInfo = vk::ImageViewCreateInfo()
                             .setImage(mImage)
                             .setFormat(format)
                             .setViewType(vk::ImageViewType::e2D)
                             .setComponents({vk::ComponentSwizzle::eIdentity,
                                             vk::ComponentSwizzle::eIdentity,
                                             vk::ComponentSwizzle::eIdentity,
                                             vk::ComponentSwizzle::eIdentity})
                             .setSubresourceRange({vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1});

    mImageView = device.Handle().createImageViewUnique(imageViewInfo);
  }

  // Transition to eGeneral and clear texture
  // TODO perhaps have initial color or data in the constructor?
  device.Execute(
      [&](vk::CommandBuffer commandBuffer)
      {
        if (memoryUsage != VMA_MEMORY_USAGE_CPU_ONLY)
        {
          Barrier(commandBuffer,
                  imageLayout,
                  vk::AccessFlagBits{},
                  vk::ImageLayout::eGeneral,
                  vk::AccessFlagBits::eMemoryRead);

          auto clearValue = vk::ClearColorValue().setFloat32({{0.0f, 0.0f, 0.0f, 0.0f}});

          commandBuffer.clearColorImage(
              mImage,
              vk::ImageLayout::eGeneral,
              clearValue,
              vk::ImageSubresourceRange{vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1});

          Barrier(commandBuffer,
                  vk::ImageLayout::eGeneral,
                  vk::AccessFlagBits::eMemoryWrite,
                  vk::ImageLayout::eGeneral,
                  vk::AccessFlagBits::eMemoryRead);
        }
        else
        {
          Barrier(commandBuffer,
                  imageLayout,
                  vk::AccessFlagBits{},
                  vk::ImageLayout::eGeneral,
                  vk::AccessFlagBits::eMemoryRead);
        }
      });
}

Texture::~Texture()
{
  if (mImage != VK_NULL_HANDLE)
  {
    vmaDestroyImage(mDevice.Allocator(), mImage, mAllocation);
  }
}

Texture::Texture(Texture&& other)
    : mDevice(other.mDevice)
    , mWidth(other.mWidth)
    , mHeight(other.mHeight)
    , mFormat(other.mFormat)
    , mImage(other.mImage)
    , mAllocation(other.mAllocation)
    , mAllocationInfo(other.mAllocationInfo)
    , mImageView(std::move(other.mImageView))
{
  other.mImage = VK_NULL_HANDLE;
  other.mAllocation = VK_NULL_HANDLE;
}

void Texture::Clear(vk::CommandBuffer commandBuffer, const std::array<int, 4>& colour)
{
  vk::ClearColorValue colourValue(colour);
  Clear(commandBuffer, colourValue);
}

void Texture::Clear(vk::CommandBuffer commandBuffer, const std::array<float, 4>& colour)
{
  vk::ClearColorValue colourValue(colour);
  Clear(commandBuffer, colourValue);
}

void Texture::Clear(vk::CommandBuffer commandBuffer, vk::ClearColorValue colour)
{
  commandBuffer.clearColorImage(
      mImage,
      vk::ImageLayout::eGeneral,
      colour,
      vk::ImageSubresourceRange{vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1});

  Barrier(commandBuffer,
          vk::ImageLayout::eGeneral,
          vk::AccessFlagBits::eMemoryWrite,
          vk::ImageLayout::eGeneral,
          vk::AccessFlagBits::eMemoryRead);
}

void Texture::CopyFrom(const void* data)
{
  vk::DeviceSize bytesPerPixel = GetBytesPerPixel(mFormat);

  // TODO use always mapped functionality of VMA

  VkMemoryPropertyFlags memFlags;
  vmaGetMemoryTypeProperties(mDevice.Allocator(), mAllocationInfo.memoryType, &memFlags);
  if ((memFlags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT) == 0)
    throw std::runtime_error("Not visible image");

  void* pData;
  if (vmaMapMemory(mDevice.Allocator(), mAllocation, &pData) != VK_SUCCESS)
    throw std::runtime_error("Cannot map buffer");

  auto subresource = vk::ImageSubresource()
                         .setAspectMask(vk::ImageAspectFlagBits::eColor)
                         .setMipLevel(0)
                         .setArrayLayer(0);

  auto srcLayout = mDevice.Handle().getImageSubresourceLayout(mImage, subresource);

  const uint8_t* src = (const uint8_t*)data;
  uint8_t* dst = (uint8_t*)pData;

  dst += srcLayout.offset;
  for (uint32_t y = 0; y < mHeight; y++)
  {
    std::memcpy(dst, src, mWidth * bytesPerPixel);

    dst += srcLayout.rowPitch;
    src += mWidth * bytesPerPixel;
  }

  if ((memFlags & VK_MEMORY_PROPERTY_HOST_COHERENT_BIT) == 0)
  {
    auto memRange = vk::MappedMemoryRange()
                        .setMemory(mAllocationInfo.deviceMemory)
                        .setOffset(mAllocationInfo.offset)
                        .setSize(mAllocationInfo.size);

    mDevice.Handle().flushMappedMemoryRanges(memRange);
  }

  vmaUnmapMemory(mDevice.Allocator(), mAllocation);
}

void Texture::CopyTo(void* data)
{
  vk::DeviceSize bytesPerPixel = GetBytesPerPixel(mFormat);

  // TODO use always mapped functionality of VMA

  VkMemoryPropertyFlags memFlags;
  vmaGetMemoryTypeProperties(mDevice.Allocator(), mAllocationInfo.memoryType, &memFlags);
  if ((memFlags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT) == 0)
    throw std::runtime_error("Not visible image");

  void* pData;
  if (vmaMapMemory(mDevice.Allocator(), mAllocation, &pData) != VK_SUCCESS)
    throw std::runtime_error("Cannot map buffer");

  if ((memFlags & VK_MEMORY_PROPERTY_HOST_COHERENT_BIT) == 0)
  {
    auto memRange = vk::MappedMemoryRange()
                        .setMemory(mAllocationInfo.deviceMemory)
                        .setOffset(mAllocationInfo.offset)
                        .setSize(mAllocationInfo.size);

    mDevice.Handle().invalidateMappedMemoryRanges(memRange);
  }

  auto subresource = vk::ImageSubresource()
                         .setAspectMask(vk::ImageAspectFlagBits::eColor)
                         .setMipLevel(0)
                         .setArrayLayer(0);

  auto srcLayout = mDevice.Handle().getImageSubresourceLayout(mImage, subresource);

  uint8_t* dst = (uint8_t*)data;
  const uint8_t* src = (const uint8_t*)pData;

  src += srcLayout.offset;
  for (uint32_t y = 0; y < mHeight; y++)
  {
    std::memcpy(dst, src, mWidth * bytesPerPixel);

    src += srcLayout.rowPitch;
    dst += mWidth * bytesPerPixel;
  }

  vmaUnmapMemory(mDevice.Allocator(), mAllocation);
}

void Texture::CopyFrom(vk::CommandBuffer commandBuffer, Texture& srcImage)
{
  if (!(mWidth == srcImage.mWidth && mHeight == srcImage.mHeight && mFormat == srcImage.mFormat))
  {
    throw std::runtime_error("Invalid source texture to copy");
  }

  srcImage.Barrier(commandBuffer,
                   vk::ImageLayout::eGeneral,
                   vk::AccessFlagBits::eShaderRead | vk::AccessFlagBits::eColorAttachmentRead,
                   vk::ImageLayout::eTransferSrcOptimal,
                   vk::AccessFlagBits::eMemoryRead);
  Barrier(commandBuffer,
          vk::ImageLayout::eGeneral,
          vk::AccessFlagBits::eShaderRead | vk::AccessFlagBits::eColorAttachmentRead,
          vk::ImageLayout::eTransferDstOptimal,
          vk::AccessFlagBits::eMemoryWrite);

  auto region = vk::ImageCopy()
                    .setSrcSubresource({vk::ImageAspectFlagBits::eColor, 0, 0, 1})
                    .setDstSubresource({vk::ImageAspectFlagBits::eColor, 0, 0, 1})
                    .setExtent({mWidth, mHeight, 1});

  commandBuffer.copyImage(srcImage.mImage,
                          vk::ImageLayout::eTransferSrcOptimal,
                          mImage,
                          vk::ImageLayout::eTransferDstOptimal,
                          region);

  srcImage.Barrier(commandBuffer,
                   vk::ImageLayout::eTransferSrcOptimal,
                   vk::AccessFlagBits::eMemoryRead,
                   vk::ImageLayout::eGeneral,
                   vk::AccessFlagBits::eShaderRead | vk::AccessFlagBits::eColorAttachmentRead |
                       vk::AccessFlagBits::eHostRead);

  Barrier(commandBuffer,
          vk::ImageLayout::eTransferDstOptimal,
          vk::AccessFlagBits::eMemoryWrite,
          vk::ImageLayout::eGeneral,
          vk::AccessFlagBits::eShaderRead | vk::AccessFlagBits::eColorAttachmentRead |
              vk::AccessFlagBits::eHostRead);
}

void Texture::Barrier(vk::CommandBuffer commandBuffer,
                      vk::ImageLayout oldLayout,
                      vk::AccessFlags srcMask,
                      vk::ImageLayout newLayout,
                      vk::AccessFlags dstMask)
{
  TextureBarrier(mImage, commandBuffer, oldLayout, srcMask, newLayout, dstMask);
}

vk::ImageView Texture::GetView() const
{
  if (!mImageView)
  {
    throw std::runtime_error("No image view created!");
  }

  return *mImageView;
}

uint32_t Texture::GetWidth() const
{
  return mWidth;
}

uint32_t Texture::GetHeight() const
{
  return mHeight;
}

vk::Format Texture::GetFormat() const
{
  return mFormat;
}

vk::Image Texture::Handle() const
{
  return mImage;
}

void TextureBarrier(vk::Image image,
                    vk::CommandBuffer commandBuffer,
                    vk::ImageLayout oldLayout,
                    vk::AccessFlags srcMask,
                    vk::ImageLayout newLayout,
                    vk::AccessFlags dstMask)
{
  auto imageMemoryBarriers = vk::ImageMemoryBarrier()
                                 .setSrcQueueFamilyIndex(VK_QUEUE_FAMILY_IGNORED)
                                 .setDstQueueFamilyIndex(VK_QUEUE_FAMILY_IGNORED)
                                 .setOldLayout(oldLayout)
                                 .setNewLayout(newLayout)
                                 .setImage(image)
                                 .setSubresourceRange({vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1})
                                 .setSrcAccessMask(srcMask)
                                 .setDstAccessMask(dstMask);

  commandBuffer.pipelineBarrier(
      vk::PipelineStageFlagBits::eAllCommands | vk::PipelineStageFlagBits::eHost,
      vk::PipelineStageFlagBits::eAllCommands | vk::PipelineStageFlagBits::eHost,
      {},
      nullptr,
      nullptr,
      imageMemoryBarriers);
}

}  // namespace Renderer
}  // namespace Vortex
