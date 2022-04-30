//
//  Buffer.h
//  Vortex
//

#pragma once

#include <Vortex/Renderer/Common.h>

namespace Vortex
{
namespace Renderer
{
class Texture;
class Device;
class CommandEncoder;

/**
 * @brief A vulkan buffer which can be on the host or the device.
 */
class GenericBuffer
{
public:
  VORTEX_API GenericBuffer(Device& device,
                           BufferUsage usageFlags,
                           MemoryUsage memoryUsage,
                           std::uint64_t deviceSize);

  VORTEX_API GenericBuffer(GenericBuffer&& other);
  VORTEX_API virtual ~GenericBuffer();

  /**
   * @brief Copy a buffer to this buffer
   * @param commandBuffer command buffer to run the copy on.
   * @param srcBuffer the source buffer.
   */
  VORTEX_API void CopyFrom(CommandEncoder& command, GenericBuffer& srcBuffer);

  /**
   * @brief Copy a texture to this buffer
   * @param commandBuffer command buffer to run the copy on.
   * @param srcTexture the source texture
   */
  VORTEX_API void CopyFrom(CommandEncoder& command, Texture& srcTexture);

  /**
   * @brief The vulkan handle
   */
  VORTEX_API Handle::Buffer Handle() const;

  /**
   * @brief The size in bytes of the buffer
   */
  VORTEX_API std::uint64_t Size() const;

  /**
   * @brief Resize the buffer. Invalidates the buffer handle
   * @param size buffer size
   */
  VORTEX_API void Resize(std::uint64_t size);

  /**
   * @brief Inserts a barrier for this buffer
   * @param commandBuffer the command buffer to run the barrier
   * @param oldAccess old access
   * @param newAccess new access
   */
  VORTEX_API void Barrier(CommandEncoder& command, Access oldAccess, Access newAccess);

  /**
   * @brief Clear the buffer with 0
   * @param commandBuffer the command buffer to clear on
   */
  VORTEX_API void Clear(CommandEncoder& command);

  /**
   * @brief copy from data to buffer
   * @param offset in the buffer
   * @param data pointer
   * @param size of data
   */
  VORTEX_API void CopyFrom(uint32_t offset, const void* data, uint32_t size);

  /**
   * @brief copy buffer to data
   * @param offset in the buffer
   * @param data pointer
   * @param size of data
   */
  VORTEX_API void CopyTo(uint32_t offset, void* data, uint32_t size);

private:
  struct Impl;
  std::unique_ptr<Impl> mImpl;
};

/**
 * @brief a vertex buffer type of buffer
 */
template <typename T>
class VertexBuffer : public GenericBuffer
{
public:
  VertexBuffer(Device& device, std::size_t size, MemoryUsage memoryUsage = MemoryUsage::Gpu)
      : GenericBuffer(device, BufferUsage::Vertex, memoryUsage, sizeof(T) * size)
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
  UniformBuffer(Device& device, MemoryUsage memoryUsage = MemoryUsage::Gpu)
      : GenericBuffer(device, BufferUsage::Uniform, memoryUsage, sizeof(T))
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
  Buffer(Device& device, std::size_t size = 1, MemoryUsage memoryUsage = MemoryUsage::Gpu)
      : GenericBuffer(device, BufferUsage::Storage, memoryUsage, sizeof(T) * size)
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
  IndirectBuffer(Device& device, MemoryUsage memoryUsage = MemoryUsage::Gpu)
      : GenericBuffer(device, BufferUsage::Indirect, memoryUsage, sizeof(T))
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
  IndexBuffer(Device& device, std::size_t size, MemoryUsage memoryUsage = MemoryUsage::Gpu)
      : GenericBuffer(device, BufferUsage::Indirect, memoryUsage, sizeof(T) * size)
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
  if (sizeof(T) > buffer.Size())
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
  if (sizeof(T) * t.size() > buffer.Size())
    throw std::runtime_error("Mismatch data size");
  buffer.CopyTo(0, t.data(), sizeof(T) * t.size());
}

/**
 * @brief Copy the content of an object to the buffer.
 */
template <template <typename> class BufferType, typename T>
void CopyFrom(BufferType<T>& buffer, const T& t)
{
  if (sizeof(T) > buffer.Size())
    throw std::runtime_error("Mismatch data size");
  buffer.CopyFrom(0, &t, sizeof(T));
}

/**
 * @brief Copy the content of a vector to the buffer
 */
template <template <typename> class BufferType, typename T>
void CopyFrom(BufferType<T>& buffer, const std::vector<T>& t)
{
  if (sizeof(T) * t.size() > buffer.Size())
    throw std::runtime_error("Mismatch data size");
  buffer.CopyFrom(0u, t.data(), static_cast<uint32_t>(sizeof(T) * t.size()));
}

}  // namespace Renderer
}  // namespace Vortex
