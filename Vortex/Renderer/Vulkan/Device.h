//
//  Device.h
//  Vortex
//

#pragma once

#include <Vortex/Renderer/BindGroup.h>
#include <Vortex/Renderer/CommandBuffer.h>
#include <Vortex/Renderer/Common.h>
#include <Vortex/Renderer/Device.h>
#include <Vortex/Renderer/Pipeline.h>
#include <map>

#include "Instance.h"

namespace Vortex
{
namespace Renderer
{
/**
 * @brief A vulkan dynamic dispatcher that checks if the function is not null.
 */
struct DynamicDispatcher : vk::DispatchLoaderBase
{
  void vkCmdDebugMarkerBeginEXT(VkCommandBuffer commandBuffer,
                                const VkDebugMarkerMarkerInfoEXT* pMarkerInfo) const;
  void vkCmdDebugMarkerEndEXT(VkCommandBuffer commandBuffer) const;

  PFN_vkCmdDebugMarkerBeginEXT mVkCmdDebugMarkerBeginEXT = nullptr;
  PFN_vkCmdDebugMarkerEndEXT mVkCmdDebugMarkerEndEXT = nullptr;
};

class VulkanDevice : public Device
{
public:
  VORTEX_API VulkanDevice(const Instance& instance, bool validation = true);
  VORTEX_API VulkanDevice(const Instance& instance, vk::SurfaceKHR surface, bool validation = true);
  VORTEX_API VulkanDevice(const Instance& instance, int familyIndex, bool surface, bool validation);

  VORTEX_API ~VulkanDevice();

  // Implementation of Device interface
  bool HasTimer() const override;

  void WaitIdle() override;

  void Execute(CommandBuffer::CommandFn commandFn) const override;

  Handle::ShaderModule CreateShaderModule(const SpirvBinary& spirv) override;

  Handle::BindGroupLayout CreateBindGroupLayout(const SPIRV::ShaderLayouts& layout) override;

  Handle::PipelineLayout CreatePipelineLayout(const SPIRV::ShaderLayouts& layout) override;

  BindGroup CreateBindGroup(const Handle::BindGroupLayout& bindGroupLayout,
                            const SPIRV::ShaderLayouts& layout,
                            const std::vector<BindingInput>& bindingInputs) override;

  Handle::Pipeline CreateGraphicsPipeline(const GraphicsPipelineDescriptor& builder,
                                          const RenderState& renderState) override;

  Handle::Pipeline CreateComputePipeline(Handle::ShaderModule shader,
                                         Handle::PipelineLayout layout,
                                         SpecConstInfo specConstInfo = {}) override;

  // Vulkan specific functions
  VmaAllocator Allocator() const;

  VORTEX_API vk::Device Handle() const;

  VORTEX_API const DynamicDispatcher& Loader() const;

  VORTEX_API vk::Queue Queue() const;

  VORTEX_API vk::PhysicalDevice GetPhysicalDevice() const;

  int GetFamilyIndex() const;

  vk::UniqueCommandBuffer CreateCommandBuffer() const;

  vk::UniqueDescriptorSet CreateDescriptorSet(vk::DescriptorSetLayout layout) const;

private:
  void CreateDescriptorPool(int size = 512);

  vk::PhysicalDevice mPhysicalDevice;
  DynamicDispatcher mLoader;
  int mFamilyIndex;
  vk::UniqueDevice mDevice;
  vk::Queue mQueue;
  vk::UniqueCommandPool mCommandPool;
  vk::UniqueDescriptorPool mDescriptorPool;
  VmaAllocator mAllocator;
  mutable std::unique_ptr<CommandBuffer> mCommandBuffer;

  std::map<const uint32_t*, vk::UniqueShaderModule> mShaders;
  std::vector<std::tuple<SPIRV::ShaderLayouts, vk::UniqueDescriptorSetLayout>>
      mDescriptorSetLayouts;
  std::vector<std::tuple<SPIRV::ShaderLayouts, vk::UniquePipelineLayout>> mPipelineLayouts;

  struct GraphicsPipelineCache
  {
    RenderState State;
    GraphicsPipelineDescriptor Graphics;
    vk::UniquePipeline Pipeline;
  };

  struct ComputePipelineCache
  {
    vk::ShaderModule Shader;
    vk::PipelineLayout Layout;
    SpecConstInfo SpecConst;
    vk::UniquePipeline Pipeline;
  };

  std::vector<GraphicsPipelineCache> mGraphicsPipelines;
  std::vector<ComputePipelineCache> mComputePipelines;
  vk::UniquePipelineCache mPipelineCache;
};

}  // namespace Renderer
}  // namespace Vortex
