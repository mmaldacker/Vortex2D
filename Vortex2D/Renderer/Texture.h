//
//  Texture.h
//  Vortex2D
//

#ifndef Vortex2d_Texture_h
#define Vortex2d_Texture_h

#include <Vortex2D/Renderer/Common.h>
#include <Vortex2D/Utils/vk_mem_alloc.h>

namespace Vortex2D { namespace Renderer {

class Device;

/**
 * @brief Factory for a vullkan sampler
 */
class SamplerBuilder
{
public:
    VORTEX2D_API SamplerBuilder();

    /**
     * @brief Mode of the sampler: repeat, clamp, etc
     * @param mode vulkan mode
     * @return *this
     */
    VORTEX2D_API SamplerBuilder& AddressMode(vk::SamplerAddressMode mode);

    /**
     * @brief Filter of the sampler: linear, nearest, etc
     * @param filter vulkan filter
     * @return *this
     */
    VORTEX2D_API SamplerBuilder& Filter(vk::Filter filter);

    /**
     * @brief Create the vulkan sampler
     * @param device vulkan device
     * @return unique sampler
     */
    VORTEX2D_API vk::UniqueSampler Create(vk::Device device);

private:
    vk::SamplerCreateInfo mSamplerInfo;
};

/**
 * @brief A texture, or in vulkan terms, an image.
 */
class Texture
{
public:
    VORTEX2D_API Texture(const Device& device,
                         uint32_t width,
                         uint32_t height,
                         vk::Format format,
                         VmaMemoryUsage memoryUsage = VMA_MEMORY_USAGE_GPU_ONLY);

    VORTEX2D_API virtual ~Texture();
    VORTEX2D_API Texture(Texture&& other);

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

    void VORTEX2D_API CopyFrom(vk::CommandBuffer commandBuffer, Texture& srcImage);

    void VORTEX2D_API Barrier(vk::CommandBuffer commandBuffer,
                 vk::ImageLayout oldLayout,
                 vk::AccessFlags oldAccess,
                 vk::ImageLayout newLayout,
                 vk::AccessFlags newAccess);

    VORTEX2D_API vk::ImageView GetView() const;
    VORTEX2D_API uint32_t GetWidth() const;
    VORTEX2D_API uint32_t GetHeight() const;
    VORTEX2D_API vk::Format GetFormat() const;

    VORTEX2D_API void Clear(vk::CommandBuffer commandBuffer, const std::array<int, 4>& colour);
    VORTEX2D_API void Clear(vk::CommandBuffer commandBuffer, const std::array<float, 4>& colour);

    friend class GenericBuffer;

private:
    void Clear(vk::CommandBuffer commandBuffer, vk::ClearColorValue colourValue);

    VORTEX2D_API void CopyFrom(const void* data, vk::DeviceSize bytesPerPixel);
    VORTEX2D_API void CopyTo(void* data, vk::DeviceSize bytesPerPixel);

    const Device& mDevice;
    uint32_t mWidth;
    uint32_t mHeight;
    vk::Format mFormat;
    VkImage mImage;
    VmaAllocation mAllocation;
    VmaAllocationInfo mAllocationInfo;
    vk::UniqueImageView mImageView;
};

}}

#endif
