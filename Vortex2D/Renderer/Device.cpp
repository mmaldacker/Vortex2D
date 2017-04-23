//
//  Device.cpp
//  Vortex2D
//

#include "Device.h"
#include <iostream>

namespace Vortex2D { namespace Renderer {

int GetFamilyIndex(vk::PhysicalDevice physicalDevice, vk::SurfaceKHR surface)
{
    int index = -1;
    const auto& familyProperties = physicalDevice.getQueueFamilyProperties();
    for (int i = 0; i < familyProperties.size(); i++)
    {
        const auto& property = familyProperties[i];
        if ((property.queueFlags & vk::QueueFlagBits::eCompute) &&
            (property.queueFlags & vk::QueueFlagBits::eGraphics) &&
             physicalDevice.getSurfaceSupportKHR(i, surface))
        {
            index = i;
        }
    }

    if (index == -1)
    {
        throw std::runtime_error("Suitable physical device not found");
    }

    return index;
}

Device::Device(vk::PhysicalDevice physicalDevice, vk::SurfaceKHR surface, bool validation)
    : mPhysicalDevice(physicalDevice)
{
    // find queue that has compute, graphics and surface support
    int index = GetFamilyIndex(physicalDevice, surface);

    float queuePriority = 1.0f;
    vk::DeviceQueueCreateInfo deviceQueueInfo;
    deviceQueueInfo
            .setQueueFamilyIndex(index)
            .setQueueCount(1)
            .setPQueuePriorities(&queuePriority);

    const std::vector<const char*> validationLayers = {"VK_LAYER_LUNARG_standard_validation"};
    const std::vector<const char*> deviceExtensions = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};

    vk::PhysicalDeviceFeatures deviceFeatures;
    vk::DeviceCreateInfo deviceInfo;
    deviceInfo
            .setQueueCreateInfoCount(1)
            .setPQueueCreateInfos(&deviceQueueInfo)
            .setPEnabledFeatures(&deviceFeatures)
            .setEnabledExtensionCount(deviceExtensions.size())
            .setPpEnabledExtensionNames(deviceExtensions.data());

    // add the validation layer if necessary
    if (validation)
    {
        deviceInfo.setEnabledLayerCount(validationLayers.size());
        deviceInfo.setPpEnabledLayerNames(validationLayers.data());
    }

    mDevice = physicalDevice.createDeviceUnique(deviceInfo);
    mQueue = mDevice->getQueue(index, 0);
}

vk::Device Device::GetDevice() const
{
    return *mDevice;
}

vk::PhysicalDevice Device::GetPhysicalDevice() const
{
    return mPhysicalDevice;
}

}}
