//
//  Device.cpp
//  Vortex2D
//

#include "Device.h"
#include <iostream>
#include <fstream>

#include <Vortex2D/Renderer/Instance.h>

namespace Vortex2D { namespace Renderer {

// TODO two functions below are duplicated
int ComputeFamilyIndex(vk::PhysicalDevice physicalDevice, vk::SurfaceKHR surface)
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

int ComputeFamilyIndex(vk::PhysicalDevice physicalDevice)
{
    int index = -1;
    const auto& familyProperties = physicalDevice.getQueueFamilyProperties();
    for (int i = 0; i < familyProperties.size(); i++)
    {
        const auto& property = familyProperties[i];
        if ((property.queueFlags & vk::QueueFlagBits::eCompute) &&
                (property.queueFlags & vk::QueueFlagBits::eGraphics))
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
{
    float queuePriority = 1.0f;
    auto deviceQueueInfo = vk::DeviceQueueCreateInfo()
            .setQueueFamilyIndex(familyIndex)
            .setQueueCount(1)
            .setPQueuePriorities(&queuePriority);

    std::vector<const char*> validationLayers = {"VK_LAYER_LUNARG_standard_validation"};
    std::vector<const char*> deviceExtensions = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};

    // make sure we request valid layers only
    auto availableLayers = physicalDevice.enumerateDeviceLayerProperties();
    RemoveInexistingLayers(validationLayers, availableLayers);

    // create queue
    auto deviceFeatures = vk::PhysicalDeviceFeatures()
            .setShaderStorageImageExtendedFormats(true);
    auto deviceInfo = vk::DeviceCreateInfo()
            .setQueueCreateInfoCount(1)
            .setPQueueCreateInfos(&deviceQueueInfo)
            .setPEnabledFeatures(&deviceFeatures)
            .setEnabledExtensionCount((uint32_t)deviceExtensions.size())
            .setPpEnabledExtensionNames(deviceExtensions.data());

    // add the validation layer if necessary
    if (validation)
    {
        deviceInfo.setEnabledLayerCount((uint32_t)validationLayers.size());
        deviceInfo.setPpEnabledLayerNames(validationLayers.data());
    }

    mDevice = physicalDevice.createDeviceUnique(deviceInfo);
    mQueue = mDevice->getQueue(familyIndex, 0);

    // create command pool
    auto commandPoolInfo = vk::CommandPoolCreateInfo()
            .setQueueFamilyIndex(familyIndex)
            .setFlags(vk::CommandPoolCreateFlagBits::eResetCommandBuffer);
    mCommandPool = mDevice->createCommandPoolUnique(commandPoolInfo);

    // create descriptor pool
    // TODO size should be configurable
    // TODO check when we allocate more than what is allowed (might get it for free already)
    std::vector<vk::DescriptorPoolSize> poolSizes;
    poolSizes.emplace_back(vk::DescriptorType::eUniformBuffer, 256);
    poolSizes.emplace_back(vk::DescriptorType::eCombinedImageSampler, 256);
    poolSizes.emplace_back(vk::DescriptorType::eStorageImage, 256);
    poolSizes.emplace_back(vk::DescriptorType::eStorageBuffer, 512);

    vk::DescriptorPoolCreateInfo descriptorPoolInfo{};
    descriptorPoolInfo.flags = vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet;
    descriptorPoolInfo.maxSets = 512;
    descriptorPoolInfo.poolSizeCount = (uint32_t)poolSizes.size();
    descriptorPoolInfo.pPoolSizes = poolSizes.data();
    mDescriptorPool = mDevice->createDescriptorPoolUnique(descriptorPoolInfo);
}

vk::Device Device::Handle() const
{
    return *mDevice;
}

vk::Queue Device::Queue() const
{
    return mQueue;
}

vk::DescriptorPool Device::DescriptorPool() const
{
    return *mDescriptorPool;
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

uint32_t Device::FindMemoryPropertiesIndex(uint32_t memoryTypeBits, vk::MemoryPropertyFlags properties) const
{
    auto memoryProperties = mPhysicalDevice.getMemoryProperties();
    for (uint32_t i = 0; i < memoryProperties.memoryTypeCount; ++i)
    {
        if ((memoryTypeBits & (1 << i)) &&
                ((memoryProperties.memoryTypes[i].propertyFlags & properties) == properties))
            return i;
    }

    throw std::runtime_error("Memory type not found");
}

vk::DescriptorSetLayout Device::CreateDescriptorSetLayout(vk::DescriptorSetLayoutCreateInfo layoutInfo) const
{
    mDescriptorSetLayouts.emplace_back(mDevice->createDescriptorSetLayoutUnique(layoutInfo));
    return *mDescriptorSetLayouts.back();
}

vk::ShaderModule Device::GetShaderModule(const std::string& filename) const
{
    auto shaderIt = mShaders.find(filename);
    if (shaderIt != mShaders.end())
    {
        return *shaderIt->second.Module;
    }

    std::ifstream is(filename, std::ios::binary | std::ios::in | std::ios::ate);
    if (!is.is_open())
    {
        throw std::runtime_error("Couldn't open file:" + filename);
    }

    std::vector<uint32_t> content;

    size_t size = is.tellg();
    is.seekg(0, std::ios::beg);
    content.resize((size + 3) / 4);
    is.read(reinterpret_cast<char*>(content.data()), size);
    is.close();

    auto shaderInfo = vk::ShaderModuleCreateInfo()
            .setCodeSize(size)
            .setPCode(content.data());

    auto shaderModule = mDevice->createShaderModuleUnique(shaderInfo);
    auto shader = *shaderModule;
    mShaders[filename] = {content, std::move(shaderModule)};
    return shader;
}

const std::vector<uint32_t> Device::GetShaderSPIRV(const std::string& filename) const
{
  GetShaderModule(filename);
  return mShaders[filename].Binary;
}

}}
