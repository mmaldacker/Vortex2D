//
//  Buffer.cpp
//  Vortex2D
//

#include "Buffer.h"

namespace Vortex2D { namespace Renderer {

template<typename T>
VertexBuffer<T>::VertexBuffer(const Device& device, const std::vector<T>& vertexBuffer)
{
    uint32_t size = sizeof(T) * vertexBuffer.size();
    vk::BufferCreateInfo bufferInfo;
    bufferInfo
            .setSize(size)
            .setUsage(vk::BufferUsageFlagBits::eVertexBuffer)
            .setSharingMode(vk::SharingMode::eExclusive);

    mBuffer = device.GetDevice().createBufferUnique(bufferInfo);

    auto memoryRequirements = device.GetDevice().getBufferMemoryRequirements(*mBuffer);

    // TODO use a staging buffer
    vk::MemoryPropertyFlags flags = vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent;
    uint32_t memoryPropertyIndex = device.FindMemoryPropertiesIndex(memoryRequirements.memoryTypeBits, flags);

    vk::MemoryAllocateInfo memoryInfo;
    memoryInfo
            .setAllocationSize(memoryRequirements.size)
            .setMemoryTypeIndex(memoryPropertyIndex);

    // TODO use a proper memory allocator
    mBufferMemory = device.GetDevice().allocateMemoryUnique(memoryInfo);
    device.GetDevice().bindBufferMemory(*mBuffer, *mBufferMemory, 0);

    void* data = device.GetDevice().mapMemory(*mBufferMemory, 0, size, vk::MemoryMapFlags());
    std::memcpy(data, vertexBuffer.data(), size);
    device.GetDevice().unmapMemory(*mBufferMemory);
}

template class VertexBuffer<float>;
template class VertexBuffer<glm::vec2>;

}}
