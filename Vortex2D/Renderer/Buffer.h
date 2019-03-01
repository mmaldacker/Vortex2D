//
//  Buffer.h
//  Vortex2D
//

#ifndef Vortex2D_Buffer_h
#define Vortex2D_Buffer_h

#include <Vortex2D/Renderer/Common.h>
#include <Vortex2D/Utils/vk_mem_alloc.h>

namespace Vortex2D
{
namespace Renderer
{
class Texture;
class Device;

/**
 * @brief A vulkan buffer which can be on the host or the device.
 */
class GenericBuffer
{
public:
  VORTEX2D_API GenericBuffer(const Device& device,
                             vk::BufferUsageFlags usageFlags,
                             VmaMemoryUsage memoryUsage,
                             vk::DeviceSize deviceSize);

  VORTEX2D_API virtual ~GenericBuffer();

  VORTEX2D_API GenericBuffer(GenericBuffer&& other);

  /**
   * @brief Copy a buffer to this buffer
   * @param commandBuffer command buffer to run the copy on.
   * @param srcBuffer the source buffer.
   */
  VORTEX2D_API void CopyFrom(vk::CommandBuffer commandBuffer, GenericBuffer& srcBuffer);

  /**
   * @brief Copy a texture to this buffer
   * @param commandBuffer command buffer to run the copy on.
   * @param srcTexture the source texture
   */
  VORTEX2D_API void CopyFrom(vk::CommandBuffer commandBuffer, Texture& srcTexture);

  /**
   * @brief The vulkan handle
   */
  VORTEX2D_API vk::Buffer Handle() const;

  /**
   * @brief The size in bytes of the buffer
   */
  VORTEX2D_API vk::DeviceSize Size() const;

  /**
   * @brief Resize the buffer. Invalidates the buffer handle
   * @param size buffer size
   */
  VORTEX2D_API void Resize(vk::DeviceSize size);

  /**
   * @brief Inserts a barrier for this buffer
   * @param commandBuffer the command buffer to run the barrier
   * @param oldAccess old access
   * @param newAccess new access
   */
  VORTEX2D_API void Barrier(vk::CommandBuffer commandBuffer,
                            vk::AccessFlags oldAccess,
                            vk::AccessFlags newAccess);

  /**
   * @brief Clear the buffer with 0
   * @param commandBuffer the command buffer to clear on
   */
  VORTEX2D_API void Clear(vk::CommandBuffer commandBuffer);

  /**
   * @brief copy from data to buffer
   * @param offset in the buffer
   * @param data pointer
   * @param size of data
   */
  VORTEX2D_API void CopyFrom(uint32_t offset, const void* data, uint32_t size);

  /**
   * @brief copy buffer to data
   * @param offset in the buffer
   * @param data pointer
   * @param size of data
   */
  VORTEX2D_API void CopyTo(uint32_t offset, void* data, uint32_t size);

protected:
  void Create();

  const Device& mDevice;
  vk::DeviceSize mSize;
  vk::BufferUsageFlags mUsageFlags;
  VmaMemoryUsage mMemoryUsage;
  VkBuffer mBuffer;
  VmaAllocation mAllocation;
  VmaAllocationInfo mAllocationInfo;
};

/**
 * @brief a vertex buffer type of buffer
 */
template <typename T>
class VertexBuffer : public GenericBuffer
{
public:
  VertexBuffer(const Device& device,
               std::size_t size,
               VmaMemoryUsage memoryUsage = VMA_MEMORY_USAGE_GPU_ONLY)
      : GenericBuffer(device, vk::BufferUsageFlagBits::eVertexBuffer, memoryUsage, sizeof(T) * size)
  {
  }
};

/**
 * @brief a uniform buffer type of buffer
 */
template <typename T>
class UniformBuffer : public GenericBuffer
{
public:
  UniformBuffer(const Device& device, VmaMemoryUsage memoryUsage = VMA_MEMORY_USAGE_GPU_ONLY)
      : GenericBuffer(device, vk::BufferUsageFlagBits::eUniformBuffer, memoryUsage, sizeof(T))
  {
  }
};

/**
 * @brief a storage buffer type of buffer
 */
template <typename T>
class Buffer : public GenericBuffer
{
public:
  Buffer(const Device& device,
         std::size_t size = 1,
         VmaMemoryUsage memoryUsage = VMA_MEMORY_USAGE_GPU_ONLY)
      : GenericBuffer(device,
                      vk::BufferUsageFlagBits::eStorageBuffer,
                      memoryUsage,
                      sizeof(T) * size)
  {
  }
};

/**
 * @brief an indirect buffer type of buffer, used for compute indirect dispatch
 */
template <typename T>
class IndirectBuffer : public GenericBuffer
{
public:
  IndirectBuffer(const Device& device, VmaMemoryUsage memoryUsage = VMA_MEMORY_USAGE_GPU_ONLY)
      : GenericBuffer(
            device,
            vk::BufferUsageFlagBits::eIndirectBuffer | vk::BufferUsageFlagBits::eStorageBuffer,
            memoryUsage,
            sizeof(T))
  {
  }
};

/**
 * @brief a index buffer type of buffer
 */
template <typename T>
class IndexBuffer : public GenericBuffer
{
public:
  IndexBuffer(const Device& device,
              std::size_t size,
              VmaMemoryUsage memoryUsage = VMA_MEMORY_USAGE_GPU_ONLY)
      : GenericBuffer(device, vk::BufferUsageFlagBits::eIndexBuffer, memoryUsage, sizeof(T) * size)
  {
    static_assert(std::is_same<uint16_t, T>::value || std::is_same<uint32_t, T>::value,
                  "IndexBuffer needs to be uint16_t or uint32_t");
  }
};

/**
 * @brief Copy the content of a buffer in an object
 */
template <template <typename> class BufferType, typename T>
void CopyTo(BufferType<T>& buffer, T& t)
{
  if (sizeof(T) != buffer.Size())
    throw std::runtime_error("Mismatch data size");
  buffer.CopyTo(0, &t, sizeof(T));
}

/**
 * @brief Copy the content of a buffer to a vector. Vector needs to have the
 * correct size already.
 */
template <template <typename> class BufferType, typename T>
void CopyTo(BufferType<T>& buffer, std::vector<T>& t)
{
  if (sizeof(T) * t.size() != buffer.Size())
    throw std::runtime_error("Mismatch data size");
  buffer.CopyTo(0, t.data(), sizeof(T) * t.size());
}

/**
 * @brief Copy the content of an object to the buffer.
 */
template <template <typename> class BufferType, typename T>
void CopyFrom(BufferType<T>& buffer, const T& t)
{
  if (sizeof(T) != buffer.Size())
    throw std::runtime_error("Mismatch data size");
  buffer.CopyFrom(0, &t, sizeof(T));
}

/**
 * @brief Copy the content of a vector to the buffer
 */
template <template <typename> class BufferType, typename T>
void CopyFrom(BufferType<T>& buffer, const std::vector<T>& t)
{
  if (sizeof(T) * t.size() != buffer.Size())
    throw std::runtime_error("Mismatch data size");
  buffer.CopyFrom(0, t.data(), sizeof(T) * t.size());
}

/**
 * @brief Inserts a barrier for the given buffer, command buffer and access.
 * @param buffer the vulkan buffer handle
 * @param commandBuffer the command buffer to inserts the barrier
 * @param oldAccess old access
 * @param newAccess new access
 */
VORTEX2D_API void BufferBarrier(vk::Buffer buffer,
                                vk::CommandBuffer commandBuffer,
                                vk::AccessFlags oldAccess,
                                vk::AccessFlags newAccess);

}  // namespace Renderer
}  // namespace Vortex2D

#endif
