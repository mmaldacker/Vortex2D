//
//  Instance.cpp
//  Vortex2D
//

#include "Instance.h"

#include <iostream>

VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugReportFlagsEXT /*flags*/,
                                             VkDebugReportObjectTypeEXT /*objType*/,
                                             uint64_t /*obj*/,
                                             size_t /*location*/,
                                             int32_t /*code*/,
                                             const char* /*layerPrefix*/,
                                             const char* msg,
                                             void* /*userData*/)
{
  std::cout << "validation layer: " << msg << std::endl;
  return VK_FALSE;
}

namespace Vortex
{
namespace Renderer
{
Instance::Instance(const std::string& name,
                   std::vector<const char*> extraExtensions,
                   bool validation)
{
  auto availableLayers = vk::enumerateInstanceLayerProperties();
  auto availableExtensions = vk::enumerateInstanceExtensionProperties();

  std::vector<const char*> layers;
  std::vector<const char*> extensions;

  // add the validation extension if necessary
  if (validation)
  {
    if (HasExtension(VK_EXT_DEBUG_REPORT_EXTENSION_NAME, availableExtensions))
    {
      extensions.push_back(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);
    }
    if (HasLayer(VK_LAYER_LUNARG_STANDARD_VALIDATION_NAME, availableLayers))
    {
      layers.push_back(VK_LAYER_LUNARG_STANDARD_VALIDATION_NAME);
    }
  }

  // add extra extensions
  for (auto& extension : extraExtensions)
  {
    if (HasExtension(extension, availableExtensions))
    {
      extensions.push_back(extension);
    }
  }

  // configure instance
  auto appInfo = vk::ApplicationInfo()
                     .setPApplicationName(name.c_str())
                     .setApiVersion(VK_MAKE_VERSION(1, 1, 0));

  auto instanceInfo = vk::InstanceCreateInfo()
                          .setPApplicationInfo(&appInfo)
                          .setEnabledExtensionCount((uint32_t)extensions.size())
                          .setPpEnabledExtensionNames(extensions.data())
                          .setEnabledLayerCount((uint32_t)layers.size())
                          .setPpEnabledLayerNames(layers.data());

  try
  {
    mInstance = vk::createInstanceUnique(instanceInfo);
  }
  catch (const vk::IncompatibleDriverError&)
  {
    // vulkan 1.1 not available, try 1.0
    appInfo.setApiVersion(VK_MAKE_VERSION(1, 0, 0));
    mInstance = vk::createInstanceUnique(instanceInfo);
  }

  // init dynamic loader
  mLoader.init(*mInstance, vkGetInstanceProcAddr, VK_NULL_HANDLE, nullptr);

  // add the validation calback if necessary
  if (validation && HasExtension(VK_EXT_DEBUG_REPORT_EXTENSION_NAME, availableExtensions))
  {
    auto debugCallbackInfo =
        vk::DebugReportCallbackCreateInfoEXT()
            .setPfnCallback(debugCallback)
            .setFlags(vk::DebugReportFlagBitsEXT::eWarning | vk::DebugReportFlagBitsEXT::eError);

    mDebugCallback = mInstance->createDebugReportCallbackEXT(debugCallbackInfo, nullptr, mLoader);
  }

  // find first discrete GPU
  std::size_t bestDeviceIndex = 0;
  auto devices = mInstance->enumeratePhysicalDevices();
  for (std::size_t i = 0; i < devices.size(); i++)
  {
    if (devices[i].getProperties().deviceType == vk::PhysicalDeviceType::eDiscreteGpu)
    {
      bestDeviceIndex = i;
    }
  }

  mPhysicalDevice = devices.at(bestDeviceIndex);
  auto properties = mPhysicalDevice.getProperties();
  std::cout << "Device name: " << properties.deviceName << std::endl;
}

Instance::~Instance()
{
  mInstance->destroyDebugReportCallbackEXT(mDebugCallback, nullptr, mLoader);
}

vk::PhysicalDevice Instance::GetPhysicalDevice() const
{
  return mPhysicalDevice;
}

vk::Instance Instance::GetInstance() const
{
  return *mInstance;
}

bool HasLayer(const char* extension, const std::vector<vk::LayerProperties>& availableExtensions)
{
  return std::any_of(availableExtensions.begin(),
                     availableExtensions.end(),
                     [&](const vk::LayerProperties& layer) {
                       return std::strcmp(extension, layer.layerName) == 0;
                     });
}

bool HasExtension(const char* extension,
                  const std::vector<vk::ExtensionProperties>& availableExtensions)
{
  return std::any_of(availableExtensions.begin(),
                     availableExtensions.end(),
                     [&](const vk::ExtensionProperties& layer) {
                       return std::strcmp(extension, layer.extensionName) == 0;
                     });
}

}  // namespace Renderer
}  // namespace Vortex
