//
//  Instance.cpp
//  Vortex2D
//

#include "Instance.h"

#include <iostream>

static PFN_vkCreateDebugReportCallbackEXT pfn_vkCreateDebugReportCallbackEXT;
VkResult vkCreateDebugReportCallbackEXT(VkInstance instance,
                                        const VkDebugReportCallbackCreateInfoEXT* pCreateInfo,
                                        const VkAllocationCallbacks* pAllocator,
                                        VkDebugReportCallbackEXT* pCallback)
{
    return pfn_vkCreateDebugReportCallbackEXT(instance, pCreateInfo, pAllocator, pCallback);
}

static PFN_vkDestroyDebugReportCallbackEXT pfn_vkDestroyDebugReportCallbackEXT;
void vkDestroyDebugReportCallbackEXT(VkInstance instance,
                                     VkDebugReportCallbackEXT callback,
                                     const VkAllocationCallbacks* pAllocator)
{
    pfn_vkDestroyDebugReportCallbackEXT(instance, callback, pAllocator);
}

VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugReportFlagsEXT flags,
                                             VkDebugReportObjectTypeEXT objType,
                                             uint64_t obj,
                                             size_t location,
                                             int32_t code,
                                             const char* layerPrefix,
                                             const char* msg,
                                             void* userData)
{
    std::cout << "validation layer: " << msg << std::endl;
    return VK_FALSE;
}

namespace Vortex2D { namespace Renderer {

void Instance::Create(const std::string& name, std::vector<const char*> extensions, bool validation)
{
    // add the validation extension if necessary
    if (validation)
    {
        extensions.push_back(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);
    }

    // configure instance
    vk::ApplicationInfo appInfo;
    appInfo.setPApplicationName(name.c_str());

    const std::vector<const char*> validationLayers = {"VK_LAYER_LUNARG_standard_validation"};

    vk::InstanceCreateInfo instanceInfo;
    instanceInfo
            .setPApplicationInfo(&appInfo)
            .setEnabledExtensionCount((uint32_t)extensions.size())
            .setPpEnabledExtensionNames(extensions.data());

    // add the validation layer if necessary
    if (validation)
    {
        instanceInfo.setEnabledLayerCount((uint32_t)validationLayers.size());
        instanceInfo.setPpEnabledLayerNames(validationLayers.data());
    }

    mInstance = vk::createInstanceUnique(instanceInfo);

    // add the validation calback if necessary
    if (validation)
    {
        pfn_vkCreateDebugReportCallbackEXT = (PFN_vkCreateDebugReportCallbackEXT) mInstance->getProcAddr("vkCreateDebugReportCallbackEXT");
        pfn_vkDestroyDebugReportCallbackEXT = (PFN_vkDestroyDebugReportCallbackEXT) mInstance->getProcAddr("vkDestroyDebugReportCallbackEXT");

        vk::DebugReportCallbackCreateInfoEXT debugCallbackInfo;
        debugCallbackInfo
                .setPfnCallback(debugCallback)
                .setFlags(vk::DebugReportFlagBitsEXT::eWarning | vk::DebugReportFlagBitsEXT::eError);

        mDebugCallback = mInstance->createDebugReportCallbackEXTUnique(debugCallbackInfo);
    }
}

vk::PhysicalDevice Instance::GetPhysicalDevice() const
{
    // TODO better search than first available device
    // - using swap chain info
    // - using queue info
    // - discrete GPU
    vk::PhysicalDevice physicalDevice = mInstance->enumeratePhysicalDevices().at(0);
    auto properties = physicalDevice.getProperties();
    std::cout << "Device name: " << properties.deviceName << std::endl;

    return physicalDevice;
}

vk::Instance Instance::GetInstance() const
{
    return *mInstance;
}

}}
