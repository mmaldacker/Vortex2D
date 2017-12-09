//
//  Buffer.h
//  Vortex2D
//

#ifndef Buffer_h
#define Buffer_h

#include <Vortex2D/Renderer/Common.h>

namespace Vortex2D { namespace Renderer {

class Texture;
class Device;

class GenericBuffer
{
public:
    GenericBuffer(const Device& device, vk::BufferUsageFlags usageFlags, bool host, vk::DeviceSize deviceSize);

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

    vk::Device mDevice;
    bool mHost;
    vk::UniqueBuffer mBuffer;
    vk::UniqueDeviceMemory mMemory;
    vk::DeviceSize mSize;
};

template<typename T>
class VertexBuffer : public GenericBuffer
{
public:
    VertexBuffer(const Device& device, std::size_t size, bool host = false)
        : GenericBuffer(device, vk::BufferUsageFlagBits::eVertexBuffer, host, sizeof(T) * size)
    {
    }
};

template<typename T>
class UniformBuffer : public GenericBuffer
{
public:
    UniformBuffer(const Device& device, bool host = false)
        : GenericBuffer(device, vk::BufferUsageFlagBits::eUniformBuffer, host, sizeof(T))
    {
    }
};

template<typename T>
class Buffer : public GenericBuffer
{
public:
    Buffer(const Device& device, std::size_t size = 1, bool host = false)
        : GenericBuffer(device, vk::BufferUsageFlagBits::eStorageBuffer, host, sizeof(T) * size)
    {
    }
};

template<typename T>
class IndirectBuffer : public GenericBuffer
{
public:
    IndirectBuffer(const Device& device, bool host = false)
        : GenericBuffer(device, vk::BufferUsageFlagBits::eIndirectBuffer | vk::BufferUsageFlagBits::eStorageBuffer, host, sizeof(T))
    {
    }
};

template<template<typename> class BufferType, typename T>
class UpdateBuffer : public BufferType<T>
{
public:
    UpdateBuffer(const Device& device)
        : BufferType<T>(device, false)
        , mLocal(device, true)
    {
    }

    void Upload(vk::CommandBuffer commandBuffer)
    {
        BufferType<T>::CopyFrom(commandBuffer, mLocal);
    }

    template<template<typename> class BufferType2, typename T2>
    friend void CopyFrom(UpdateBuffer<BufferType2, T2>&, const T2&);

    template<template<typename> class BufferType2, typename T2>
    friend void CopyTo(UpdateBuffer<BufferType2, T2>&, T2&);

private:
    BufferType<T> mLocal;
};

template<typename T>
using UpdateUniformBuffer = UpdateBuffer<UniformBuffer, T>;

template<template<typename T> class BufferType, typename T>
class UpdateBufferVector : public BufferType<T>
{
public:
    UpdateBufferVector(const Device& device, std::size_t size = 1)
        : BufferType<T>(device, size, false)
        , mLocal(device, size, true)
    {
    }

    void Upload(vk::CommandBuffer commandBuffer)
    {
        GenericBuffer::CopyFrom(commandBuffer, mLocal);
    }

    void Download(vk::CommandBuffer commandBuffer)
    {
        mLocal.CopyFrom(commandBuffer, *this);
    }

    template<template<typename> class BufferType2, typename T2>
    friend void CopyFrom(UpdateBufferVector<BufferType2, T2>&, const std::vector<T2>&);

    template<template<typename> class BufferType2, typename T2>
    friend void CopyTo(UpdateBufferVector<BufferType2, T2>&, std::vector<T2>&);

    template<template<typename> class BufferType2, typename T2>
    friend void CopyFrom(UpdateBufferVector<BufferType2, T2>&, const T2&);

    template<template<typename> class BufferType2, typename T2>
    friend void CopyTo(UpdateBufferVector<BufferType2, T2>&, T2&);

private:
    BufferType<T> mLocal;
};

template<typename T>
using UpdateVertexBuffer = UpdateBufferVector<VertexBuffer, T>;

template<typename T>
using UpdateStorageBuffer = UpdateBufferVector<Buffer, T>;

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

// UpdateBuffer functions
template<template<typename T> class BufferType, typename T>
void CopyTo(UpdateBuffer<BufferType, T>& buffer, T& t)
{
    if (sizeof(T) != buffer.Size()) throw std::runtime_error("Mismatch data size");
    CopyTo(buffer.mLocal, t);
}

template<template<typename> class BufferType, typename T>
void CopyFrom(UpdateBuffer<BufferType, T>& buffer, const T& t)
{
    if (sizeof(T) != buffer.Size()) throw std::runtime_error("Mismatch data size");
    CopyFrom(buffer.mLocal, t);
}

// UpdateBufferVector functions
template<template<typename> class BufferType, typename T>
void CopyTo(UpdateBufferVector<BufferType, T>& buffer, std::vector<T>& t)
{
    if (sizeof(T) * t.size() != buffer.Size()) throw std::runtime_error("Mismatch data size");
    CopyTo(buffer.mLocal, t);
}

template<template<typename> class BufferType, typename T>
void CopyFrom(UpdateBufferVector<BufferType, T>& buffer, const std::vector<T>& t)
{
    if (sizeof(T) * t.size() != buffer.Size()) throw std::runtime_error("Mismatch data size");
    CopyFrom(buffer.mLocal, t);
}

template<template<typename> class BufferType, typename T>
void CopyFrom(UpdateBufferVector<BufferType, T>& buffer, const T& t)
{
    if (sizeof(T) != buffer.Size()) throw std::runtime_error("Mismatch data size");
    CopyFrom(buffer.mLocal, t);
}

template<template<typename> class BufferType, typename T>
void CopyTo(UpdateBufferVector<BufferType, T>& buffer, T& t)
{
    if (sizeof(T) != buffer.Size()) throw std::runtime_error("Mismatch data size");
    CopyTo(buffer.mLocal, t);
}

void BufferBarrier(vk::Buffer buffer, vk::CommandBuffer commandBuffer, vk::AccessFlags oldAccess, vk::AccessFlags newAccess);

}}

#endif
