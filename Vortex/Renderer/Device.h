//
//  Device.h
//  Vortex
//

#pragma once

#include <Vortex/Renderer/BindGroup.h>
#include <Vortex/Renderer/CommandBuffer.h>
#include <Vortex/Renderer/Common.h>
#include <Vortex/Renderer/Pipeline.h>
#include <map>

namespace Vortex
{
namespace Renderer
{
/**
 * @brief Encapsulation around the vulkan device. Allows to create command
 * buffers, layout, bindings, memory and shaders.
 */
class Device
{
public:
  VORTEX_API Device() = default;
  VORTEX_API ~Device() = default;

  Device(Device&&) = delete;
  Device& operator=(Device&&) = delete;

  VORTEX_API virtual void WaitIdle() = 0;

  VORTEX_API virtual bool HasTimer() const = 0;

  VORTEX_API virtual void Execute(CommandBuffer::CommandFn commandFn) const = 0;
  VORTEX_API virtual Handle::ShaderModule CreateShaderModule(const SpirvBinary& spirv) = 0;

  /**
   * @brief Create, cache and return a descriptor layout given the pipeline
   * layout
   * @param layout pipeline layout
   * @return cached descriptor set layout
   */
  VORTEX_API virtual Handle::BindGroupLayout CreateBindGroupLayout(
      const SPIRV::ShaderLayouts& layout) = 0;

  /**
   * @brief create, cache and return a vulkan pipeline layout given the layout
   * @param layout pipeline layout
   * @return vulkan pipeline layout
   */
  VORTEX_API virtual Handle::PipelineLayout CreatePipelineLayout(
      const SPIRV::ShaderLayouts& layout) = 0;

  VORTEX_API virtual BindGroup CreateBindGroup(const Handle::BindGroupLayout& bindGroupLayout,
                                               const SPIRV::ShaderLayouts& layout,
                                               const std::vector<BindingInput>& bindingInputs) = 0;

  /**
   * @brief Create a graphics pipeline
   * @param builder
   * @param renderState
   * @return
   */
  VORTEX_API virtual Handle::Pipeline CreateGraphicsPipeline(
      const GraphicsPipelineDescriptor& builder,
      const RenderState& renderState) = 0;

  /**
   * @brief Create a compute pipeline
   * @param shader
   * @param layout
   * @param specConstInfo
   */
  VORTEX_API virtual Handle::Pipeline CreateComputePipeline(Handle::ShaderModule shader,
                                                            Handle::PipelineLayout layout,
                                                            SpecConstInfo specConstInfo = {}) = 0;
};

}  // namespace Renderer
}  // namespace Vortex
