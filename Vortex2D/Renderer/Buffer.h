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
    Buffer(const Device& device,
           vk::BufferUsageFlags usageFlags,
           vk::MemoryPropertyFlags memoryFlags,
           vk::DeviceSize deviceSize);

    template<typename T>
    void CopyTo(const T& data)
    {
        void* mapped = mDevice.mapMemory(*mMemory, 0, mSize, vk::MemoryMapFlagBits());
        memcpy(mapped, &data, sizeof(T));
        mDevice.unmapMemory(*mMemory);
    }

    template<typename T>
    void CopyTo(const std::vector<T>& data)
    {
        void* mapped = mDevice.mapMemory(*mMemory, 0, mSize, vk::MemoryMapFlagBits());
        memcpy(mapped, data.data(), sizeof(T) * data.size());
        mDevice.unmapMemory(*mMemory);
    }

    void Flush();

    operator vk::Buffer() const;
    vk::DeviceSize Size() const;

private:
    vk::Device mDevice;
    vk::UniqueBuffer mBuffer;
    vk::UniqueDeviceMemory mMemory;
    vk::DeviceSize mSize;
};

}}

#endif
