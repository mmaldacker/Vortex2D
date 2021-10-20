//
//  Device.h
//  Vortex
//

#pragma once

#include <Vortex/Renderer/CommandBuffer.h>
#include <Vortex/Renderer/Common.h>
#include <Vortex/Renderer/DescriptorSet.h>
#include <Vortex/Renderer/Instance.h>
#include <Vortex/Renderer/Pipeline.h>
#include <Vortex/Utils/vk_mem_alloc.h>
#include <map>

namespace Vortex
{
namespace Renderer
{
/**
 * @brief A binary SPIRV shader, to be feed to vulkan.
 */
class SpirvBinary
{
public:
  template <std::size_t N>
  SpirvBinary(const uint32_t (&spirv)[N]) : mData(spirv), mSize(N * 4)
  {
  }

  const uint32_t* data() const { return mData; }

  std::size_t size() const { return mSize; }

  std::size_t words() const { return mSize / 4; }

private:
  const uint32_t* mData;
  std::size_t mSize;
};

/**
 * @brief A vulkan dynamic dispatcher that checks if the function is not null.
 */
struct DynamicDispatcher
{
  void vkCmdDebugMarkerBeginEXT(VkCommandBuffer commandBuffer,
                                const VkDebugMarkerMarkerInfoEXT* pMarkerInfo) const;
  void vkCmdDebugMarkerEndEXT(VkCommandBuffer commandBuffer) const;

  PFN_vkCmdDebugMarkerBeginEXT mVkCmdDebugMarkerBeginEXT = nullptr;
  PFN_vkCmdDebugMarkerEndEXT mVkCmdDebugMarkerEndEXT = nullptr;
};

/**
 * @brief Encapsulation around the vulkan device. Allows to create command
 * buffers, layout, bindings, memory and shaders.
 */
class Device
{
public:
  VORTEX_API Device(const Instance& instance, bool validation = true);
  VORTEX_API Device(const Instance& instance, vk::SurfaceKHR surface, bool validation = true);
  VORTEX_API Device(const Instance& instance, int familyIndex, bool surface, bool validation);
  VORTEX_API ~Device();

  Device(Device&&) = delete;
  Device& operator=(Device&&) = delete;

  // Vulkan handles and helpers
  VORTEX_API vk::Device Handle() const;
  VORTEX_API vk::Queue Queue() const;
  VORTEX_API const DynamicDispatcher& Loader() const;
  VORTEX_API vk::PhysicalDevice GetPhysicalDevice() const;
  VORTEX_API int GetFamilyIndex() const;

  // Command buffer functions
  VORTEX_API vk::CommandBuffer CreateCommandBuffer() const;
  VORTEX_API void FreeCommandBuffer(vk::CommandBuffer commandBuffer) const;
  VORTEX_API void Execute(CommandBuffer::CommandFn commandFn) const;

  // Memory allocator
  VORTEX_API VmaAllocator Allocator() const;
  VORTEX_API LayoutManager& GetLayoutManager();
  VORTEX_API PipelineCache& GetPipelineCache();
  VORTEX_API vk::ShaderModule GetShaderModule(const SpirvBinary& spirv);

private:
  vk::PhysicalDevice mPhysicalDevice;
  DynamicDispatcher mLoader;
  int mFamilyIndex;
  vk::UniqueDevice mDevice;
  vk::Queue mQueue;
  vk::UniqueCommandPool mCommandPool;
  vk::UniqueDescriptorPool mDescriptorPool;
  VmaAllocator mAllocator;

  std::unique_ptr<CommandBuffer> mCommandBuffer;
  std::map<const uint32_t*, vk::UniqueShaderModule> mShaders;
  LayoutManager mLayoutManager;
  PipelineCache mPipelineCache;
};

}  // namespace Renderer
}  // namespace Vortex
