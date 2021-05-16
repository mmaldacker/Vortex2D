//
//  Device.cpp
//  Vortex2D
//

#include "Device.h"

#include <fstream>
#include <iostream>

#include <Vortex2D/Renderer/Instance.h>

#define VMA_IMPLEMENTATION
#include <Vortex2D/Utils/vk_mem_alloc.h>

namespace Vortex
{
namespace Renderer
{
namespace
{
int ComputeFamilyIndex(vk::PhysicalDevice physicalDevice, vk::SurfaceKHR surface = nullptr)
{
  int index = -1;
  const auto& familyProperties = physicalDevice.getQueueFamilyProperties();
  for (std::size_t i = 0; i < familyProperties.size(); i++)
  {
    const auto& property = familyProperties[i];
    if ((property.queueFlags & vk::QueueFlagBits::eCompute) &&
        (property.queueFlags & vk::QueueFlagBits::eGraphics) &&
        (!surface || physicalDevice.getSurfaceSupportKHR(static_cast<uint32_t>(i), surface)))
    {
      index = static_cast<int32_t>(i);
      break;
    }
  }

  if (index == -1)
  {
    throw std::runtime_error("Suitable physical device not found");
  }

  return index;
}
}  // namespace

void DynamicDispatcher::vkCmdDebugMarkerBeginEXT(
    VkCommandBuffer commandBuffer,
    const VkDebugMarkerMarkerInfoEXT* pMarkerInfo) const
{
  if (mVkCmdDebugMarkerBeginEXT != nullptr)
  {
    mVkCmdDebugMarkerBeginEXT(commandBuffer, pMarkerInfo);
  }
}

void DynamicDispatcher::vkCmdDebugMarkerEndEXT(VkCommandBuffer commandBuffer) const
{
  if (mVkCmdDebugMarkerEndEXT != nullptr)
  {
    mVkCmdDebugMarkerEndEXT(commandBuffer);
  }
}

Device::Device(const Instance& instance, bool validation)
    : Device(instance, ComputeFamilyIndex(instance.GetPhysicalDevice()), false, validation)
{
}

Device::Device(const Instance& instance, vk::SurfaceKHR surface, bool validation)
    : Device(instance, ComputeFamilyIndex(instance.GetPhysicalDevice(), surface), true, validation)
{
}

Device::Device(const Instance& instance, int familyIndex, bool surface, bool validation)
    : mPhysicalDevice(instance.GetPhysicalDevice())
    , mFamilyIndex(familyIndex)
    , mLayoutManager(*this)
    , mPipelineCache(*this)
{
  float queuePriority = 1.0f;
  auto deviceQueueInfo = vk::DeviceQueueCreateInfo()
                             .setQueueFamilyIndex(familyIndex)
                             .setQueueCount(1)
                             .setPQueuePriorities(&queuePriority);

  std::vector<const char*> deviceExtensions;
  std::vector<const char*> validationLayers;

  if (surface)
  {
    deviceExtensions.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
  }

  // make sure we request valid layers only
  auto availableLayers = mPhysicalDevice.enumerateDeviceLayerProperties();
  auto availableExtensions = mPhysicalDevice.enumerateDeviceExtensionProperties();

  // add validation extensions and layers
  if (validation)
  {
    if (HasLayer(VK_LAYER_LUNARG_STANDARD_VALIDATION_NAME, availableLayers))
    {
      validationLayers.push_back(VK_LAYER_LUNARG_STANDARD_VALIDATION_NAME);
    }
    if (HasExtension(VK_EXT_DEBUG_MARKER_EXTENSION_NAME, availableExtensions))
    {
      deviceExtensions.push_back(VK_EXT_DEBUG_MARKER_EXTENSION_NAME);
    }
  }

  // create queue
  auto deviceFeatures = vk::PhysicalDeviceFeatures().setShaderStorageImageExtendedFormats(true);
  auto deviceInfo = vk::DeviceCreateInfo()
                        .setQueueCreateInfoCount(1)
                        .setPQueueCreateInfos(&deviceQueueInfo)
                        .setPEnabledFeatures(&deviceFeatures)
                        .setEnabledExtensionCount((uint32_t)deviceExtensions.size())
                        .setPpEnabledExtensionNames(deviceExtensions.data())
                        .setEnabledLayerCount((uint32_t)validationLayers.size())
                        .setPpEnabledLayerNames(validationLayers.data());

  mDevice = mPhysicalDevice.createDeviceUnique(deviceInfo);
  mQueue = mDevice->getQueue(familyIndex, 0);

  // load marker ext
  if (HasExtension(VK_EXT_DEBUG_MARKER_EXTENSION_NAME, availableExtensions))
  {
    mLoader.mVkCmdDebugMarkerBeginEXT =
        (PFN_vkCmdDebugMarkerBeginEXT)vkGetDeviceProcAddr(*mDevice, "vkCmdDebugMarkerBeginEXT");
    mLoader.mVkCmdDebugMarkerEndEXT =
        (PFN_vkCmdDebugMarkerEndEXT)vkGetDeviceProcAddr(*mDevice, "vkCmdDebugMarkerEndEXT");
  }

  // create command pool
  auto commandPoolInfo = vk::CommandPoolCreateInfo()
                             .setQueueFamilyIndex(familyIndex)
                             .setFlags(vk::CommandPoolCreateFlagBits::eResetCommandBuffer);
  mCommandPool = mDevice->createCommandPoolUnique(commandPoolInfo);

  // create alllocator
  VmaAllocatorCreateInfo allocatorInfo = {};
  allocatorInfo.physicalDevice = mPhysicalDevice;
  allocatorInfo.device = *mDevice;

  if (vmaCreateAllocator(&allocatorInfo, &mAllocator) != VK_SUCCESS)
  {
    throw std::runtime_error("Error creating allocator");
  }

  // create objects depending on device
  mLayoutManager.CreateDescriptorPool();
  mPipelineCache.CreateCache();
  mCommandBuffer = std::make_unique<CommandBuffer>(*this, true);
}

Device::~Device()
{
  vmaDestroyAllocator(mAllocator);
}

vk::Device Device::Handle() const
{
  return *mDevice;
}

vk::Queue Device::Queue() const
{
  return mQueue;
}

const DynamicDispatcher& Device::Loader() const
{
  return mLoader;
}

LayoutManager& Device::GetLayoutManager() const
{
  return mLayoutManager;
}

PipelineCache& Device::GetPipelineCache() const
{
  return mPipelineCache;
}

vk::PhysicalDevice Device::GetPhysicalDevice() const
{
  return mPhysicalDevice;
}

int Device::GetFamilyIndex() const
{
  return mFamilyIndex;
}

vk::CommandBuffer Device::CreateCommandBuffer() const
{
  auto commandBufferInfo = vk::CommandBufferAllocateInfo()
                               .setCommandBufferCount(1)
                               .setCommandPool(*mCommandPool)
                               .setLevel(vk::CommandBufferLevel::ePrimary);

  return mDevice->allocateCommandBuffers(commandBufferInfo).at(0);
}

void Device::FreeCommandBuffer(vk::CommandBuffer commandBuffer) const
{
  mDevice->freeCommandBuffers(*mCommandPool, {commandBuffer});
}

void Device::Execute(CommandBuffer::CommandFn commandFn) const
{
  (*mCommandBuffer).Record(commandFn).Submit().Wait();
}

VmaAllocator Device::Allocator() const
{
  return mAllocator;
}

vk::ShaderModule Device::GetShaderModule(const SpirvBinary& spirv) const
{
  auto it = mShaders.find(spirv.data());
  if (it != mShaders.end())
  {
    return *it->second;
  }

  if (spirv.size() == 0)
    throw std::runtime_error("Invalid SPIRV");

  auto shaderInfo = vk::ShaderModuleCreateInfo().setCodeSize(spirv.size()).setPCode(spirv.data());

  auto shaderModule = mDevice->createShaderModuleUnique(shaderInfo);
  auto shader = *shaderModule;
  mShaders[spirv.data()] = std::move(shaderModule);
  return shader;
}

}  // namespace Renderer
}  // namespace Vortex2D
