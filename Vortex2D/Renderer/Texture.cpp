//
//  Texture.cpp
//  Vortex2D
//

#include "Texture.h"

#include <cassert>

namespace Vortex2D { namespace Renderer {

SamplerBuilder::SamplerBuilder()
{
    // TODO add sampler configuration
    mSamplerInfo = vk::SamplerCreateInfo()
            .setMaxAnisotropy(1.0f);
}

vk::UniqueSampler SamplerBuilder::Create(vk::Device device)
{
    return device.createSamplerUnique(mSamplerInfo);
}

Texture::Texture(const Device& device, uint32_t width, uint32_t height, vk::Format format, bool host)
    : mHost(host)
    , mDevice(device.Handle())
    , mWidth(width)
    , mHeight(height)
{
    auto formatProperties = device.GetPhysicalDevice().getFormatProperties(format);
    assert(formatProperties.optimalTilingFeatures & vk::FormatFeatureFlagBits::eStorageImage);

    mLayout = host ? vk::ImageLayout::ePreinitialized : vk::ImageLayout::eUndefined;
    mAccess = host ? vk::AccessFlagBits::eHostWrite | vk::AccessFlagBits::eTransferWrite : vk::AccessFlagBits{};

    // TODO perhaps only set transferSrc or transferDst depending on how it's used?
    vk::ImageUsageFlags usageFlags = vk::ImageUsageFlagBits::eTransferSrc |
                                     vk::ImageUsageFlagBits::eTransferDst;

    if (!host)
    {
        // TODO color attachement only for render textures
        usageFlags |= vk::ImageUsageFlagBits::eSampled |
                      vk::ImageUsageFlagBits::eStorage |
                      vk::ImageUsageFlagBits::eColorAttachment;
    }

    auto imageInfo = vk::ImageCreateInfo()
            .setImageType(vk::ImageType::e2D)
            .setExtent({width, height, 1})
            .setMipLevels(1)
            .setArrayLayers(1)
            .setFormat(format)
            .setTiling(host ? vk::ImageTiling::eLinear : vk::ImageTiling::eOptimal)
            .setInitialLayout(mLayout)
            .setUsage(usageFlags)
            .setSharingMode(vk::SharingMode::eExclusive)
            .setSamples(vk::SampleCountFlagBits::e1);

    mImage = device.Handle().createImageUnique(imageInfo);

    vk::MemoryRequirements requirements = device.Handle().getImageMemoryRequirements(*mImage);

    auto memoryFlags = host ? vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent :
                              vk::MemoryPropertyFlagBits::eDeviceLocal;

    auto allocateInfo = vk::MemoryAllocateInfo()
            .setAllocationSize(requirements.size)
            .setMemoryTypeIndex(device.FindMemoryPropertiesIndex(requirements.memoryTypeBits, memoryFlags));

    mMemory = device.Handle().allocateMemoryUnique(allocateInfo);
    device.Handle().bindImageMemory(*mImage, *mMemory, 0);

    if (!host)
    {
        auto imageViewInfo = vk::ImageViewCreateInfo()
                .setImage(*mImage)
                .setFormat(format)
                .setViewType(vk::ImageViewType::e2D)
                .setComponents({vk::ComponentSwizzle::eIdentity,
                                vk::ComponentSwizzle::eIdentity,
                                vk::ComponentSwizzle::eIdentity,
                                vk::ComponentSwizzle::eIdentity})
                .setSubresourceRange({vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1});

        mImageView = device.Handle().createImageViewUnique(imageViewInfo);
    }
}

void Texture::CopyFrom(const void* data, vk::DeviceSize bytesPerPixel)
{
    if (mHost)
    {
        auto subresource = vk::ImageSubresource()
                .setAspectMask(vk::ImageAspectFlagBits::eColor)
                .setMipLevel(0)
                .setArrayLayer(0);

        auto srcLayout = mDevice.getImageSubresourceLayout(*mImage, subresource);

        const uint8_t* src = (const uint8_t *)data;
        uint8_t* dst = (uint8_t *)mDevice.mapMemory(*mMemory, 0, VK_WHOLE_SIZE, {});

        dst += srcLayout.offset;
        for (uint32_t y = 0; y < mHeight; y++)
        {
            std::memcpy(dst, src, mWidth * bytesPerPixel);

            dst += srcLayout.rowPitch;
            src += mWidth * bytesPerPixel;
        }
        mDevice.unmapMemory(*mMemory);
    }
}

void Texture::CopyTo(void* data, vk::DeviceSize bytesPerPixel)
{
    if (mHost)
    {
        auto subresource = vk::ImageSubresource()
                .setAspectMask(vk::ImageAspectFlagBits::eColor)
                .setMipLevel(0)
                .setArrayLayer(0);

        auto srcLayout = mDevice.getImageSubresourceLayout(*mImage, subresource);

        uint8_t* dst = (uint8_t *)data;
        const uint8_t* src = (const uint8_t *)mDevice.mapMemory(*mMemory, 0, VK_WHOLE_SIZE, {});

        src +=  srcLayout.offset;
        for (uint32_t y = 0; y < mHeight; y++)
        {
            std::memcpy(dst, src, mWidth * bytesPerPixel);

            src += srcLayout.rowPitch;
            dst += mWidth * bytesPerPixel;
        }
        mDevice.unmapMemory(*mMemory);
    }
}

void Texture::CopyFrom(vk::CommandBuffer commandBuffer, Texture& srcImage)
{
    srcImage.Barrier(commandBuffer, vk::ImageLayout::eTransferSrcOptimal, vk::AccessFlagBits::eTransferRead);
    Barrier(commandBuffer, vk::ImageLayout::eTransferDstOptimal, vk::AccessFlagBits::eTransferWrite);

    auto region = vk::ImageCopy()
            .setSrcSubresource({vk::ImageAspectFlagBits::eColor, 0, 0, 1})
            .setDstSubresource({vk::ImageAspectFlagBits::eColor, 0, 0, 1})
            .setExtent({mWidth, mHeight, 0});

    commandBuffer.copyImage(*srcImage.mImage,
                            vk::ImageLayout::eTransferSrcOptimal,
                            *mImage,
                            vk::ImageLayout::eTransferDstOptimal,
                            region);
}

void Texture::Barrier(vk::CommandBuffer commandBuffer, vk::ImageLayout newLayout, vk::AccessFlags newAccess)
{
    if (newLayout == mLayout && newAccess == mAccess)
    {
        return;
    }

    vk::ImageLayout oldLayout = mLayout;
    mLayout = newLayout;
    vk::AccessFlags oldAccss = mAccess;
    mAccess = newAccess;

    auto imageMemoryBarriers = vk::ImageMemoryBarrier()
            .setSrcQueueFamilyIndex(VK_QUEUE_FAMILY_IGNORED)
            .setDstQueueFamilyIndex(VK_QUEUE_FAMILY_IGNORED)
            .setOldLayout(oldLayout)
            .setNewLayout(newLayout)
            .setImage(*mImage)
            .setSubresourceRange({vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1})
            .setSrcAccessMask(oldAccss)
            .setDstAccessMask(newAccess);

    commandBuffer.pipelineBarrier(vk::PipelineStageFlagBits::eAllCommands,
                                  vk::PipelineStageFlagBits::eAllCommands,
                                  {},
                                  nullptr,
                                  nullptr,
                                  imageMemoryBarriers);
}

Texture::operator vk::ImageView() const
{
    return *mImageView;
}

uint32_t Texture::Width() const
{
    return mWidth;
}

uint32_t Texture::Height() const
{
    return mHeight;
}

}}
