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
    Device(vk::Device device, vk::Queue queue);

private:
    vk::Device mDevice;
    vk::Queue mQueue;
};

}}

#endif
