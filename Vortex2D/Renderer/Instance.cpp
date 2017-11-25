//
//  Instance.cpp
//  Vortex2D
//

#include "Instance.h"

#include <vk_loader/vk_loader.h>
#include <iostream>

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
    // load symbols
    if (!vkLoaderInit()) throw std::runtime_error("cannot load vulkan library!");

    // add the validation extension if necessary
    if (validation)
    {
        extensions.push_back(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);
    }

    // configure instance
    auto appInfo = vk::ApplicationInfo()
            .setPApplicationName(name.c_str())
            .setApiVersion(VK_MAKE_VERSION(1, 0, 65));

    std::vector<const char*> validationLayers = {"VK_LAYER_LUNARG_standard_validation"};

    // make sure we request available layers
    auto availableLayers = vk::enumerateInstanceLayerProperties();
    RemoveInexistingLayers(validationLayers, availableLayers);

    auto availableExtensions = vk::enumerateInstanceExtensionProperties();
    RemoveInexistingExtensions(extensions, availableExtensions);

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

    // load symbols
    if (!vkLoaderInstanceInit(*mInstance)) throw std::runtime_error("cannot load instance procs");

    // add the validation calback if necessary
    if (validation)
    {
        vk::DebugReportCallbackCreateInfoEXT debugCallbackInfo;
        debugCallbackInfo
                .setPfnCallback(debugCallback)
                .setFlags(vk::DebugReportFlagBitsEXT::eWarning | vk::DebugReportFlagBitsEXT::eError);

        mDebugCallback = mInstance->createDebugReportCallbackEXTUnique(debugCallbackInfo);
    }

    // get physical device
    // TODO better search than first available device
    // - using swap chain info
    // - using queue info
    // - discrete GPU
    mPhysicalDevice = mInstance->enumeratePhysicalDevices().at(0);
    auto properties = mPhysicalDevice.getProperties();
    std::cout << "Device name: " << properties.deviceName << std::endl;

}

vk::PhysicalDevice Instance::GetPhysicalDevice() const
{
    return mPhysicalDevice;
}

vk::Instance Instance::GetInstance() const
{
    return *mInstance;
}

void RemoveInexistingLayers(std::vector<const char*>& list, const std::vector<vk::LayerProperties>& available)
{
    for (auto it = list.begin(); it != list.end();)
    {
        auto find_it = std::find_if(available.begin(), available.end(),
            [&](const vk::LayerProperties& layer)
            {
                return std::strcmp(*it, layer.layerName) == 0;
            });

        if (find_it == available.end())
        {
            it = list.erase(it);
        }
        else
        {
            it++;
        }
    }
}

void RemoveInexistingExtensions(std::vector<const char*>& list, const std::vector<vk::ExtensionProperties>& available)
{
    for (auto it = list.begin(); it != list.end();)
    {
        auto find_it = std::find_if(available.begin(), available.end(),
            [&](const vk::ExtensionProperties& layer)
            {
                return std::strcmp(*it, layer.extensionName) == 0;
            });

        if (find_it == available.end())
        {
            it = list.erase(it);
        }
        else
        {
            it++;
        }
    }
}

}}
