//
//  Device.cpp
//  Vortex2D
//

#include "Device.h"

#include <iostream>
#include <fstream>

#include <Vortex2D/Renderer/Instance.h>

#define VMA_IMPLEMENTATION
#include <Vortex2D/Utils/vk_mem_alloc.h>

static PFN_vkCmdDebugMarkerBeginEXT vortex2d_vkCmdDebugMarkerBeginEXT = nullptr;
static PFN_vkCmdDebugMarkerEndEXT vortex2d_vkCmdDebugMarkerEndEXT = nullptr;

VKAPI_ATTR void VKAPI_CALL vkCmdDebugMarkerBeginEXT(VkCommandBuffer   commandBuffer,const  VkDebugMarkerMarkerInfoEXT *  pMarkerInfo)
{
    if (vortex2d_vkCmdDebugMarkerBeginEXT)
    {
        vortex2d_vkCmdDebugMarkerBeginEXT(commandBuffer,pMarkerInfo);
    }
}

VKAPI_ATTR void VKAPI_CALL vkCmdDebugMarkerEndEXT(VkCommandBuffer   commandBuffer)
{
    if (vortex2d_vkCmdDebugMarkerEndEXT)
    {
        vortex2d_vkCmdDebugMarkerEndEXT(commandBuffer);
    }
}

namespace Vortex2D { namespace Renderer {

namespace
{

// TODO two functions below are duplicated
int ComputeFamilyIndex(vk::PhysicalDevice physicalDevice, vk::SurfaceKHR surface)
{
    int index = -1;
    const auto& familyProperties = physicalDevice.getQueueFamilyProperties();
    for (std::size_t i = 0; i < familyProperties.size(); i++)
    {
        const auto& property = familyProperties[i];
        if ((property.queueFlags & vk::QueueFlagBits::eCompute) &&
                (property.queueFlags & vk::QueueFlagBits::eGraphics) &&
                physicalDevice.getSurfaceSupportKHR(static_cast<uint32_t>(i), surface))
        {
            index = static_cast<int32_t>(i);
        }
    }

    if (index == -1)
    {
        throw std::runtime_error("Suitable physical device not found");
    }

    return index;
}

int ComputeFamilyIndex(vk::PhysicalDevice physicalDevice)
{
    int32_t index = -1;
    const auto& familyProperties = physicalDevice.getQueueFamilyProperties();
    for (std::size_t i = 0; i < familyProperties.size(); i++)
    {
        const auto& property = familyProperties[i];
        if ((property.queueFlags & vk::QueueFlagBits::eCompute) &&
                (property.queueFlags & vk::QueueFlagBits::eGraphics))
        {
            index = static_cast<int32_t>(i);
        }
    }

    if (index == -1)
    {
        throw std::runtime_error("Suitable physical device not found");
    }

    return index;
}

}

Device::Device(vk::PhysicalDevice physicalDevice, bool validation)
    : Device(physicalDevice, ComputeFamilyIndex(physicalDevice), validation)
{
}

Device::Device(vk::PhysicalDevice physicalDevice, vk::SurfaceKHR surface, bool validation)
    : Device(physicalDevice, ComputeFamilyIndex(physicalDevice, surface), validation)
{
}

Device::Device(vk::PhysicalDevice physicalDevice, int familyIndex, bool validation)
    : mPhysicalDevice(physicalDevice)
    , mFamilyIndex(familyIndex)
    , mLayoutManager(*this)
{
    float queuePriority = 1.0f;
    auto deviceQueueInfo = vk::DeviceQueueCreateInfo()
            .setQueueFamilyIndex(familyIndex)
            .setQueueCount(1)
            .setPQueuePriorities(&queuePriority);

    // this should always be available
    std::vector<const char*> deviceExtensions = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};
    std::vector<const char*> validationLayers;

    // make sure we request valid layers only
    auto availableLayers = physicalDevice.enumerateDeviceLayerProperties();
    auto availableExtensions = physicalDevice.enumerateDeviceExtensionProperties();

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
    auto deviceFeatures = vk::PhysicalDeviceFeatures()
            .setShaderStorageImageExtendedFormats(true);
    auto deviceInfo = vk::DeviceCreateInfo()
            .setQueueCreateInfoCount(1)
            .setPQueueCreateInfos(&deviceQueueInfo)
            .setPEnabledFeatures(&deviceFeatures)
            .setEnabledExtensionCount((uint32_t)deviceExtensions.size())
            .setPpEnabledExtensionNames(deviceExtensions.data())
            .setEnabledLayerCount((uint32_t)validationLayers.size())
            .setPpEnabledLayerNames(validationLayers.data());

    mDevice = physicalDevice.createDeviceUnique(deviceInfo);
    mQueue = mDevice->getQueue(familyIndex, 0);

    // load marker ext
    if (HasExtension(VK_EXT_DEBUG_MARKER_EXTENSION_NAME, availableExtensions))
    {
        vortex2d_vkCmdDebugMarkerBeginEXT = (PFN_vkCmdDebugMarkerBeginEXT) vkGetDeviceProcAddr(*mDevice, "vkCmdDebugMarkerBeginEXT");
        vortex2d_vkCmdDebugMarkerEndEXT = (PFN_vkCmdDebugMarkerEndEXT) vkGetDeviceProcAddr(*mDevice, "vkCmdDebugMarkerEndEXT");
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

    // create descriptor pool
    mLayoutManager.CreateDescriptorPool();
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

LayoutManager& Device::GetLayoutManager() const
{
    return mLayoutManager;
}

vk::PhysicalDevice Device::GetPhysicalDevice() const
{
    return mPhysicalDevice;
}

int Device::GetFamilyIndex() const
{
    return mFamilyIndex;
}

std::vector<vk::CommandBuffer> Device::CreateCommandBuffers(uint32_t size) const
{
    auto commandBufferInfo = vk::CommandBufferAllocateInfo()
            .setCommandBufferCount(size)
            .setCommandPool(*mCommandPool)
            .setLevel(vk::CommandBufferLevel::ePrimary);

    return mDevice->allocateCommandBuffers(commandBufferInfo);
}

void Device::FreeCommandBuffers(vk::ArrayProxy<const vk::CommandBuffer> commandBuffers) const
{
    mDevice->freeCommandBuffers(*mCommandPool, commandBuffers);
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

    if (spirv.size() == 0) throw std::runtime_error("Invalid SPIRV");

    auto shaderInfo = vk::ShaderModuleCreateInfo()
            .setCodeSize(spirv.size())
            .setPCode(spirv.data());

    auto shaderModule = mDevice->createShaderModuleUnique(shaderInfo);
    auto shader = *shaderModule;
    mShaders[spirv.data()] = std::move(shaderModule);
    return shader;
}

}}
