//
//  Buffer.h
//  Vortex2D
//

#ifndef Buffer_h
#define Buffer_h

#include <Vortex2D/Renderer/Common.h>
#include <Vortex2D/Utils/vk_mem_alloc.h>

namespace Vortex2D { namespace Renderer {

class Texture;
class Device;

class GenericBuffer
{
public:
    GenericBuffer(const Device& device,
                  vk::BufferUsageFlags usageFlags,
                  VmaMemoryUsage memoryUsage,
                  vk::DeviceSize deviceSize);

    virtual ~GenericBuffer();

    GenericBuffer(GenericBuffer&& other);

    void CopyFrom(vk::CommandBuffer commandBuffer, GenericBuffer& srcBuffer);
    void CopyFrom(vk::CommandBuffer commandBuffer, Texture& srcTexture);

    vk::Buffer Handle() const;
    vk::DeviceSize Size() const;

    void Barrier(vk::CommandBuffer commandBuffer, vk::AccessFlags oldAccess, vk::AccessFlags newAccess);

    void Clear(vk::CommandBuffer commandBuffer);

    // Template friend functions for copying to and from buffers
    template<template<typename> class BufferType, typename T>
    friend void CopyFrom(BufferType<T>&, const T&);
    template<template<typename> class BufferType, typename T>
    friend void CopyTo(BufferType<T>&, T&);

    template<template<typename> class BufferType, typename T>
    friend void CopyTo(BufferType<T>&, std::vector<T>&);
    template<template<typename> class BufferType, typename T>
    friend void CopyFrom(BufferType<T>&, const std::vector<T>&);

protected:
    void CopyFrom(const void* data);
    void CopyTo(void* data);

    const Device& mDevice;
    vk::DeviceSize mSize;
    VkBuffer mBuffer;
    VmaAllocation mAllocation;
    VmaAllocationInfo mAllocationInfo;
};

template<typename T>
class VertexBuffer : public GenericBuffer
{
public:
    VertexBuffer(const Device& device, std::size_t size, VmaMemoryUsage memoryUsage = VMA_MEMORY_USAGE_GPU_ONLY)
        : GenericBuffer(device, vk::BufferUsageFlagBits::eVertexBuffer, memoryUsage, sizeof(T) * size)
    {
    }
};

template<typename T>
class UniformBuffer : public GenericBuffer
{
public:
    UniformBuffer(const Device& device, VmaMemoryUsage memoryUsage = VMA_MEMORY_USAGE_GPU_ONLY)
        : GenericBuffer(device, vk::BufferUsageFlagBits::eUniformBuffer, memoryUsage, sizeof(T))
    {
    }
};

template<typename T>
class Buffer : public GenericBuffer
{
public:
    Buffer(const Device& device, std::size_t size = 1, VmaMemoryUsage memoryUsage = VMA_MEMORY_USAGE_GPU_ONLY)
        : GenericBuffer(device, vk::BufferUsageFlagBits::eStorageBuffer, memoryUsage, sizeof(T) * size)
    {
    }
};

template<typename T>
class IndirectBuffer : public GenericBuffer
{
public:
    IndirectBuffer(const Device& device, VmaMemoryUsage memoryUsage = VMA_MEMORY_USAGE_GPU_ONLY)
        : GenericBuffer(device, vk::BufferUsageFlagBits::eIndirectBuffer | vk::BufferUsageFlagBits::eStorageBuffer, memoryUsage, sizeof(T))
    {
    }
};

template<template<typename> class BufferType, typename T>
void CopyTo(BufferType<T>& buffer, T& t)
{
    if (sizeof(T) != buffer.Size()) throw std::runtime_error("Mismatch data size");
    buffer.CopyTo(&t);
}

template<template<typename> class BufferType, typename T>
void CopyTo(BufferType<T>& buffer, std::vector<T>& t)
{
    if (sizeof(T) * t.size() != buffer.Size()) throw std::runtime_error("Mismatch data size");
    buffer.CopyTo(t.data());
}

template<template<typename> class BufferType, typename T>
void CopyFrom(BufferType<T>& buffer, const T& t)
{
    if (sizeof(T) != buffer.Size()) throw std::runtime_error("Mismatch data size");
    buffer.CopyFrom(&t);
}

template<template<typename> class BufferType, typename T>
void CopyFrom(BufferType<T>& buffer, const std::vector<T>& t)
{
    if (sizeof(T) * t.size() != buffer.Size()) throw std::runtime_error("Mismatch data size");
    buffer.CopyFrom(t.data());
}

void BufferBarrier(vk::Buffer buffer, vk::CommandBuffer commandBuffer, vk::AccessFlags oldAccess, vk::AccessFlags newAccess);

}}

#endif
