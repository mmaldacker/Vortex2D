//
//  Device.cpp
//  Vortex2D
//

#include "Device.h"
#include <iostream>

namespace Vortex2D { namespace Renderer {

// TODO two functions below are duplicated
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

int GetFamilyIndex(vk::PhysicalDevice physicalDevice)
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
    : Device(physicalDevice, GetFamilyIndex(physicalDevice), validation)
{
}

Device::Device(vk::PhysicalDevice physicalDevice, vk::SurfaceKHR surface, bool validation)
    : Device(physicalDevice, GetFamilyIndex(physicalDevice, surface), validation)
{
}

Device::Device(vk::PhysicalDevice physicalDevice, int familyIndex, bool validation)
    : mPhysicalDevice(physicalDevice)
{
    float queuePriority = 1.0f;
    auto deviceQueueInfo = vk::DeviceQueueCreateInfo()
            .setQueueFamilyIndex(familyIndex)
            .setQueueCount(1)
            .setPQueuePriorities(&queuePriority);

    const std::vector<const char*> validationLayers = {"VK_LAYER_LUNARG_standard_validation"};
    const std::vector<const char*> deviceExtensions = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};

    // create queue
    vk::PhysicalDeviceFeatures deviceFeatures;
    auto deviceInfo = vk::DeviceCreateInfo()
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
    poolSizes.emplace_back(vk::DescriptorType::eUniformBuffer, 128);
    poolSizes.emplace_back(vk::DescriptorType::eCombinedImageSampler, 128);
    poolSizes.emplace_back(vk::DescriptorType::eStorageImage, 128);
    poolSizes.emplace_back(vk::DescriptorType::eStorageBuffer, 128);

    vk::DescriptorPoolCreateInfo descriptorPoolInfo{};
    descriptorPoolInfo.flags = vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet;
    descriptorPoolInfo.maxSets = 256;
    descriptorPoolInfo.poolSizeCount = poolSizes.size();
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

// TODO replace with command buffer ring
// TODO use barrier instead of fence
template<typename F>
void ExecuteCommand(F f)
{
    vk::CommandBuffer cmd = CreateCommandBuffers(1).at(0);

    auto cmdBeginInfo = vk::CommandBufferBeginInfo()
            .setFlags(vk::CommandBufferUsageFlagBits::eOneTimeSubmit);

    cmd.begin(cmdBeginInfo);
    f(cmd);
    cmd.end();

    auto submitInfo = vk::SubmitInfo()
            .setCommandBufferCount(1)
            .setPCommandBuffers(&cmd);

    mQueue.submit({submitInfo}, nullptr);
    mQueue.waitIdle();
    FreeCommandBuffers({cmd});
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

vk::ShaderModule Device::CreateShaderModule(vk::ShaderModuleCreateInfo moduleInfo) const
{
    mShaders.emplace_back(mDevice->createShaderModuleUnique(moduleInfo));
    return *mShaders.back();
}

}}
