//
//  Texture.h
//  Vortex
//

#pragma once

#include <Vortex/Renderer/Common.h>
#include <Vortex/Utils/vk_mem_alloc.h>

namespace Vortex
{
namespace Renderer
{
class Device;

/**
 * @brief Gets the number of bytes per pixel given the format
 * @param format of texture
 * @return bytes per pixel
 */
VORTEX_API vk::DeviceSize GetBytesPerPixel(vk::Format format);

/**
 * @brief Factory for a vullkan sampler
 */
class SamplerBuilder
{
public:
  VORTEX_API SamplerBuilder();

  /**
   * @brief Mode of the sampler: repeat, clamp, etc
   * @param mode vulkan mode
   * @return *this
   */
  VORTEX_API SamplerBuilder& AddressMode(vk::SamplerAddressMode mode);

  /**
   * @brief Filter of the sampler: linear, nearest, etc
   * @param filter vulkan filter
   * @return *this
   */
  VORTEX_API SamplerBuilder& Filter(vk::Filter filter);

  /**
   * @brief Create the vulkan sampler
   * @param device vulkan device
   * @return unique sampler
   */
  VORTEX_API vk::UniqueSampler Create(vk::Device device);

private:
  vk::SamplerCreateInfo mSamplerInfo;
};

/**
 * @brief A texture, or in vulkan terms, an image.
 */
class Texture
{
public:
  VORTEX_API Texture(Device& device,
                     uint32_t width,
                     uint32_t height,
                     vk::Format format,
                     VmaMemoryUsage memoryUsage = VMA_MEMORY_USAGE_GPU_ONLY);
  VORTEX_API Texture(Texture&& other);

  VORTEX_API virtual ~Texture();

  template <typename T>
  void CopyFrom(const std::vector<T>& data)
  {
    if (data.size() != mWidth * mHeight)
      throw std::runtime_error("Invalid input data size");
    if (sizeof(T) != GetBytesPerPixel(mFormat))
      throw std::runtime_error("Invalid input data format");
    CopyFrom(data.data());
  }

  template <typename T>
  void CopyTo(std::vector<T>& data)
  {
    if (data.size() != mWidth * mHeight)
      throw std::runtime_error("Invalid input data size");
    if (sizeof(T) != GetBytesPerPixel(mFormat))
      throw std::runtime_error("Invalid input data format");
    CopyTo(data.data());
  }

  /**
   * @brief Copies width*heigh*bytesPerPixel amount of data
   * @param data source data
   */
  VORTEX_API void CopyFrom(const void* data);

  /**
   * @brief Copies width*heigh*bytesPerPixel amount of data
   * @param data destination data
   */
  VORTEX_API void CopyTo(void* data);

  /**
   * @brief Copies source texture in this texture
   * @param commandBuffer vulkan command buffer
   * @param srcImage source image
   */
  VORTEX_API void CopyFrom(vk::CommandBuffer commandBuffer, Texture& srcImage);

  VORTEX_API void Barrier(vk::CommandBuffer commandBuffer,
                          vk::ImageLayout oldLayout,
                          vk::AccessFlags oldAccess,
                          vk::ImageLayout newLayout,
                          vk::AccessFlags newAccess);

  VORTEX_API vk::ImageView GetView() const;
  VORTEX_API uint32_t GetWidth() const;
  VORTEX_API uint32_t GetHeight() const;
  VORTEX_API vk::Format GetFormat() const;

  VORTEX_API void Clear(vk::CommandBuffer commandBuffer, const std::array<int, 4>& colour);
  VORTEX_API void Clear(vk::CommandBuffer commandBuffer, const std::array<float, 4>& colour);

  VORTEX_API vk::Image Handle() const;

  friend class GenericBuffer;

private:
  void Clear(vk::CommandBuffer commandBuffer, vk::ClearColorValue colourValue);
  Device& mDevice;
  uint32_t mWidth;
  uint32_t mHeight;
  vk::Format mFormat;
  VkImage mImage;
  VmaAllocation mAllocation;
  VmaAllocationInfo mAllocationInfo;
  vk::UniqueImageView mImageView;
};

/**
 * @brief Inserts a barrier for the given texture, command buffer and access.
 * @param image the vulkan image handle
 * @param commandBuffer the vulkan command buffer
 * @param oldLayout old layout
 * @param srcMask old access
 * @param newLayout new layout
 * @param dstMask new access
 */
VORTEX_API void TextureBarrier(vk::Image image,
                               vk::CommandBuffer commandBuffer,
                               vk::ImageLayout oldLayout,
                               vk::AccessFlags srcMask,
                               vk::ImageLayout newLayout,
                               vk::AccessFlags dstMask);

}  // namespace Renderer
}  // namespace Vortex
