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
    void CopyFrom(const T& src)
    {
        assert(sizeof(T) == mSize);

        void* mapped = mDevice.mapMemory(*mMemory, 0, mSize, vk::MemoryMapFlagBits());
        std::memcpy(mapped, &src, mSize);
        mDevice.unmapMemory(*mMemory);
    }

    template<typename T>
    void CopyFrom(const std::vector<T>& src)
    {
        assert(sizeof(T) * src.size() == mSize);

        void* mapped = mDevice.mapMemory(*mMemory, 0, mSize, vk::MemoryMapFlagBits());
        std::memcpy(mapped, src.data(), mSize);
        mDevice.unmapMemory(*mMemory);
    }

    template<typename T>
    void CopyTo(std::vector<T>& dst)
    {
        assert(sizeof(T) * dst.size() == mSize);

        void* mapped = mDevice.mapMemory(*mMemory, 0, mSize, vk::MemoryMapFlagBits());
        std::memcpy(dst.data(), mapped, mSize);
        mDevice.unmapMemory(*mMemory);
    }

    void CopyFrom(vk::CommandBuffer commandBuffer, Buffer& srcBuffer);

    void Flush();

    operator vk::Buffer() const;
    vk::DeviceSize Size() const;

    void Barrier(vk::CommandBuffer commandBuffer, vk::AccessFlags newAccess);

    // TODO add clear function

private:
    vk::Device mDevice;
    vk::UniqueBuffer mBuffer;
    vk::UniqueDeviceMemory mMemory;
    vk::DeviceSize mSize;
    vk::AccessFlags mAccess;
};

}}

#endif
