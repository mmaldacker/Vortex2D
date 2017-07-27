//
//  Texture.cpp
//  Vortex2D
//

#include "Texture.h"

#include <Vortex2D/Renderer/CommandBuffer.h>

#include <cassert>

namespace Vortex2D { namespace Renderer {

SamplerBuilder::SamplerBuilder()
{
    // TODO add sampler configuration
    mSamplerInfo = vk::SamplerCreateInfo()
            .setMaxAnisotropy(1.0f);
}

SamplerBuilder& SamplerBuilder::AddressMode(vk::SamplerAddressMode mode)
{
    mSamplerInfo
            .setAddressModeU(mode)
            .setAddressModeV(mode);
    return *this;
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

    // TODO perhaps only set transferSrc or transferDst depending on how it's used?
    vk::ImageUsageFlags usageFlags = vk::ImageUsageFlagBits::eTransferSrc |
                                     vk::ImageUsageFlagBits::eTransferDst;

    vk::ImageLayout imageLayout = host ? vk::ImageLayout::ePreinitialized : vk::ImageLayout::eUndefined;

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
            .setInitialLayout(imageLayout)
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

    // Transition to eGeneral and clear texture
    // TODO perhaps have initial color or data in the constructor?
    // TODO put in clear command
    CommandBuffer cmd(device);
    cmd.Record([&](vk::CommandBuffer commandBuffer)
    {
        Barrier(commandBuffer,
                imageLayout,
                vk::AccessFlagBits{},
                vk::ImageLayout::eGeneral,
                vk::AccessFlagBits::eMemoryRead);

        auto clearValue = vk::ClearColorValue()
                .setFloat32(std::array<float, 4>{{0.0f, 0.0f, 0.0f, 0.0f}});

        commandBuffer.clearColorImage(*mImage,
                                      vk::ImageLayout::eGeneral,
                                      clearValue,
                                      vk::ImageSubresourceRange{vk::ImageAspectFlagBits::eColor, 0, 1, 0 ,1});
    });
    cmd.Submit();
    cmd.Wait();
}

void Texture::CopyFrom(const void* data, vk::DeviceSize bytesPerPixel)
{
    // TODO verify bytesPerPixel is correct

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
    if (mWidth != srcImage.mWidth || mHeight != srcImage.mHeight)
    {
        return;
    }

    // TODO too many barrier maybe?

    srcImage.Barrier(commandBuffer,
                     vk::ImageLayout::eGeneral,
                     vk::AccessFlagBits::eShaderRead | vk::AccessFlagBits::eColorAttachmentRead,
                     vk::ImageLayout::eTransferSrcOptimal,
                     vk::AccessFlagBits::eTransferRead);
    Barrier(commandBuffer,
            vk::ImageLayout::eGeneral,
            vk::AccessFlagBits::eShaderRead | vk::AccessFlagBits::eColorAttachmentRead,
            vk::ImageLayout::eTransferDstOptimal,
            vk::AccessFlagBits::eTransferWrite);

    auto region = vk::ImageCopy()
            .setSrcSubresource({vk::ImageAspectFlagBits::eColor, 0, 0, 1})
            .setDstSubresource({vk::ImageAspectFlagBits::eColor, 0, 0, 1})
            .setExtent({mWidth, mHeight, 0});

    commandBuffer.copyImage(*srcImage.mImage,
                            vk::ImageLayout::eTransferSrcOptimal,
                            *mImage,
                            vk::ImageLayout::eTransferDstOptimal,
                            region);

    srcImage.Barrier(commandBuffer,
                     vk::ImageLayout::eTransferSrcOptimal,
                     vk::AccessFlagBits::eTransferRead,
                     vk::ImageLayout::eGeneral,
                     vk::AccessFlagBits::eShaderRead | vk::AccessFlagBits::eColorAttachmentRead);

    Barrier(commandBuffer,
            vk::ImageLayout::eTransferDstOptimal,
            vk::AccessFlagBits::eTransferWrite,
            vk::ImageLayout::eGeneral,
            vk::AccessFlagBits::eShaderRead | vk::AccessFlagBits::eColorAttachmentRead);
}

void Texture::Barrier(vk::CommandBuffer commandBuffer,
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
            .setImage(*mImage)
            .setSubresourceRange({vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1})
            .setSrcAccessMask(srcMask)
            .setDstAccessMask(dstMask);

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

uint32_t Texture::GetWidth() const
{
    return mWidth;
}

uint32_t Texture::GetHeight() const
{
    return mHeight;
}

}}
