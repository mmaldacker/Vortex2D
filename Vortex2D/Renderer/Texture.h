//
//  Texture.h
//  Vortex2D
//

#ifndef Vortex_Texture_h
#define Vortex_Texture_h

#include <Vortex2D/Renderer/Common.h>
#include <Vortex2D/Renderer/Device.h>

namespace Vortex2D { namespace Renderer {
class SamplerBuilder
{
public:
    SamplerBuilder();

    vk::UniqueSampler Create(vk::Device device);

private:
    vk::SamplerCreateInfo mSamplerInfo;
};

class Texture
{
public:
    Texture(const Device& device, uint32_t width, uint32_t height, vk::Format format, bool host);

    // TODO have bytesPerPixel deduced from format
    void CopyFrom(const void* data, vk::DeviceSize bytesPerPixel);
    void CopyTo(void* data, vk::DeviceSize bytesPerPixel);

    void CopyFrom(vk::CommandBuffer commandBuffer, Texture& srcImage);

    void Barrier(vk::CommandBuffer commandBuffer, vk::ImageLayout newLayout, vk::AccessFlags newAccess);
    operator vk::ImageView() const;

    uint32_t GetWidth() const;
    uint32_t GetHeight() const;

private:
    bool mHost;
    vk::Device mDevice;
    uint32_t mWidth;
    uint32_t mHeight;
    vk::UniqueImage mImage;
    vk::UniqueDeviceMemory mMemory;
    vk::UniqueImageView mImageView;
    vk::ImageLayout mLayout;
    vk::AccessFlags mAccess;
};

}}

#endif
