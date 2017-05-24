//
//  Buffer.h
//  Vortex2D
//

#ifndef Buffer_h
#define Buffer_h

#include <Vortex2D/Renderer/Common.h>
#include <Vortex2D/Renderer/Device.h>

namespace Vortex2D { namespace Renderer {

class Buffer
{
public:
    Buffer(const Device& device, vk::BufferUsageFlags usageFlags, bool host, vk::DeviceSize deviceSize);

    template<typename T>
    void CopyTo(const T& data)
    {
        assert(sizeof(T) == mSize);

        void* mapped = mDevice.mapMemory(*mMemory, 0, mSize, vk::MemoryMapFlagBits());
        std::memcpy(mapped, &data, mSize);
        mDevice.unmapMemory(*mMemory);
    }

    template<typename T>
    void CopyTo(const std::vector<T>& data)
    {
        assert(sizeof(T) * data.size() == mSize);

        void* mapped = mDevice.mapMemory(*mMemory, 0, mSize, vk::MemoryMapFlagBits());
        std::memcpy(mapped, data.data(), mSize);
        mDevice.unmapMemory(*mMemory);
    }

    void CopyFrom(vk::CommandBuffer commandBuffer, Buffer& srcBuffer);

    void Flush();

    operator vk::Buffer() const;
    vk::DeviceSize Size() const;

    void Barrier(vk::CommandBuffer commandBuffer, vk::AccessFlags newAccess);

private:
    vk::Device mDevice;
    vk::UniqueBuffer mBuffer;
    vk::UniqueDeviceMemory mMemory;
    vk::DeviceSize mSize;
    vk::AccessFlags mAccess;
};

}}

#endif
