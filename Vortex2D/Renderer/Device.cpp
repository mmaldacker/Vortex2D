//
//  Device.cpp
//  Vortex2D
//

#include "Device.h"
#include <iostream>

namespace Vortex2D { namespace Renderer {

Device::Device(vk::Device device, vk::Queue queue)
    : mDevice(device)
    , mQueue(queue)
{


}


}}
