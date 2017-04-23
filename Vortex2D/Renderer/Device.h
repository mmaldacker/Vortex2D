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

private:
    vk::PhysicalDevice mPhysicalDevice;
    vk::UniqueDevice mDevice;
    vk::Queue mQueue;
};

}}

#endif
