//
//  Texture.h
//  Vortex
//

#pragma once

#include <Vortex/Renderer/Common.h>

#include <stdexcept>

namespace Vortex
{
namespace Renderer
{
class Device;
class CommandEncoder;
class GenericBuffer;

/**
 * @brief Gets the number of bytes per pixel given the format
 * @param format of texture
 * @return bytes per pixel
 */
inline std::uint64_t GetBytesPerPixel(Format format)
{
  switch (format)
  {
    case Format::R8Uint:
    case Format::R8Sint:
      return 1;
    case Format::R32Sfloat:
    case Format::R32Sint:
    case Format::R8G8B8A8Unorm:
    case Format::B8G8R8A8Unorm:
      return 4;
    case Format::R32G32Sfloat:
      return 8;
    case Format::R32G32B32A32Sfloat:
      return 16;
    default:
      throw std::runtime_error("unsupported format");
  }
}

inline uint32_t GetPaddedBytes(uint32_t offset, uint32_t align)
{
  auto padding = (align - (offset % align)) % align;
  return offset + padding;
}

class Sampler
{
public:
  enum class AddressMode
  {
    ClampToEdge,
    Repeat,
  };

  enum class Filter
  {
    Linear,
    Nearest,
  };

  Sampler(Device& device,
          AddressMode adressMode = AddressMode::Repeat,
          Filter filter = Filter::Nearest);

  Sampler(Sampler&& other);

  VORTEX_API ~Sampler();

  Handle::Sampler Handle();

private:
  struct Impl;
  std::unique_ptr<Impl> mImpl;
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
                     Format format,
                     MemoryUsage memoryUsage = MemoryUsage::Gpu);
  VORTEX_API Texture(Texture&& other);

  VORTEX_API virtual ~Texture();

  template <typename T>
  void CopyFrom(const std::vector<T>& data)
  {
    if (data.size() > GetWidth() * GetHeight())
      throw std::runtime_error("Invalid input data size");
    if (sizeof(T) != GetBytesPerPixel(GetFormat()))
      throw std::runtime_error("Invalid input data format");
    CopyFrom(data.data(), data.size());
  }

  template <typename T>
  void CopyTo(std::vector<T>& data)
  {
    if (data.size() > GetWidth() * GetHeight())
      throw std::runtime_error("Invalid input data size");
    if (sizeof(T) != GetBytesPerPixel(GetFormat()))
      throw std::runtime_error("Invalid input data format");
    CopyTo(data.data(), data.size());
  }

  /**
   * @brief Copies width*heigh*bytesPerPixel amount of data
   * @param data source data
   */
  VORTEX_API void CopyFrom(const void* data, int size);

  /**
   * @brief Copies width*heigh*bytesPerPixel amount of data
   * @param data destination data
   */
  VORTEX_API void CopyTo(void* data, int size);

  /**
   * @brief Copies source texture in this texture
   * @param commandBuffer command buffer
   * @param srcImage source image
   */
  VORTEX_API void CopyFrom(CommandEncoder& command, Texture& srcImage);

  /**
   * @brief Copies source buffer in this texture
   * @param command command buffer
   * @param srcBuffer source buffer
   */
  VORTEX_API void CopyFrom(CommandEncoder& command, GenericBuffer& srcBuffer);

  /**
   * @brief Inserts a barrier for the given texture, command buffer and access.
   * @param image the vulkan image handle
   * @param commandBuffer the vulkan command buffer
   * @param oldLayout old layout
   * @param srcMask old access
   * @param newLayout new layout
   * @param dstMask new access
   */
  VORTEX_API void Barrier(CommandEncoder& command,
                          ImageLayout oldLayout,
                          Access oldAccess,
                          ImageLayout newLayout,
                          Access newAccess);

  VORTEX_API Handle::ImageView GetView() const;
  VORTEX_API uint32_t GetWidth() const;
  VORTEX_API uint32_t GetHeight() const;
  VORTEX_API Format GetFormat() const;

  VORTEX_API void Clear(CommandEncoder& command, const std::array<int, 4>& colour);
  VORTEX_API void Clear(CommandEncoder& command, const std::array<float, 4>& colour);

  VORTEX_API Handle::Image Handle() const;

private:
  struct Impl;
  std::unique_ptr<Impl> mImpl;
};

}  // namespace Renderer
}  // namespace Vortex
