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

    SamplerBuilder& AddressMode(vk::SamplerAddressMode mode);
    SamplerBuilder& Filter(vk::Filter filter);

    vk::UniqueSampler Create(vk::Device device);

private:
    vk::SamplerCreateInfo mSamplerInfo;
};

class Texture
{
public:
    Texture(const Device& device, uint32_t width, uint32_t height, vk::Format format, bool host);

    template<typename T>
    void CopyFrom(const std::vector<T>& data)
    {
        assert(data.size() == mWidth * mHeight);
        CopyFrom(data.data(), sizeof(T));
    }

    template<typename T>
    void CopyTo(std::vector<T>& data)
    {
        assert(data.size() == mWidth * mHeight);
        CopyTo(data.data(), sizeof(T));
    }

    void CopyFrom(vk::CommandBuffer commandBuffer, Texture& srcImage);

    void Barrier(vk::CommandBuffer commandBuffer,
                 vk::ImageLayout oldLayout,
                 vk::AccessFlags oldAccess,
                 vk::ImageLayout newLayout,
                 vk::AccessFlags newAccess);

    vk::ImageView GetView() const;
    uint32_t GetWidth() const;
    uint32_t GetHeight() const;
    vk::Format GetFormat() const;

    void Clear(vk::CommandBuffer commandBuffer, const std::array<int, 4>& colour);
    void Clear(vk::CommandBuffer commandBuffer, const std::array<float, 4>& colour);

    friend class GenericBuffer;

private:
    void Clear(vk::CommandBuffer commandBuffer, vk::ClearColorValue colourValue);

    void CopyFrom(const void* data, vk::DeviceSize bytesPerPixel);
    void CopyTo(void* data, vk::DeviceSize bytesPerPixel);

    bool mHost;
    vk::Device mDevice;
    uint32_t mWidth;
    uint32_t mHeight;
    vk::Format mFormat;
    vk::UniqueImage mImage;
    vk::UniqueDeviceMemory mMemory;
    vk::UniqueImageView mImageView;
};

}}

#endif
