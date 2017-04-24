//
//  Device.h
//  Vortex2D
//

#ifndef Device_h
#define Device_h

#include <Vortex2D/Renderer/Common.h>

namespace Vortex2D { namespace Renderer {

class Device
{
public:
    // TODO should be able to create a Device without surface (offscreen rendering)
    Device(vk::PhysicalDevice physicalDevice, vk::SurfaceKHR surface, bool validation = true);

    vk::Device GetDevice() const;
    vk::PhysicalDevice GetPhysicalDevice() const;

    std::vector<vk::CommandBuffer> CreateCommandBuffers(uint32_t size) const;
    uint32_t FindMemoryPropertiesIndex(uint32_t memoryTypeBits, vk::MemoryPropertyFlags properties) const;

private:
    vk::PhysicalDevice mPhysicalDevice;
    vk::UniqueDevice mDevice;
    vk::Queue mQueue;
    vk::UniqueCommandPool mCommandPool;
};

}}

#endif
