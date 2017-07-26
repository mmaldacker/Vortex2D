//
//  Device.h
//  Vortex2D
//

#ifndef Device_h
#define Device_h

#include <Vortex2D/Renderer/Common.h>

#include <map>

namespace Vortex2D { namespace Renderer {

class Device
{
public:
    Device(vk::PhysicalDevice physicalDevice, bool validation = true);
    Device(vk::PhysicalDevice physicalDevice, vk::SurfaceKHR surface, bool validation = true);
    Device(vk::PhysicalDevice physicalDevice, int familyIndex, bool validation = true);

    vk::Device Handle() const;
    vk::Queue Queue() const;
    vk::DescriptorPool DescriptorPool() const;
    vk::PhysicalDevice GetPhysicalDevice() const;
    std::vector<vk::CommandBuffer> CreateCommandBuffers(uint32_t size) const;
    void FreeCommandBuffers(vk::ArrayProxy<const vk::CommandBuffer> commandBuffers) const;
    uint32_t FindMemoryPropertiesIndex(uint32_t memoryTypeBits, vk::MemoryPropertyFlags properties) const;

    // TODO have a getter like shader module
    // use a map of layout description to the handle (equality should be easy)
    vk::DescriptorSetLayout CreateDescriptorSetLayout(vk::DescriptorSetLayoutCreateInfo layoutInfo) const;

    vk::ShaderModule GetShaderModule(const std::string& filename) const;

private:
    vk::PhysicalDevice mPhysicalDevice;
    vk::UniqueDevice mDevice;
    vk::Queue mQueue;
    vk::UniqueCommandPool mCommandPool;
    vk::UniqueDescriptorPool mDescriptorPool;
    mutable std::map<std::string, vk::UniqueShaderModule> mShaders;
    mutable std::vector<vk::UniqueDescriptorSetLayout> mDescriptorSetLayouts;
};

}}

#endif
