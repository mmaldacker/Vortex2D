//
//  Device.h
//  Vortex2D
//

#ifndef Vortex2d_Vulkan_Device_h
#define Vortex2d_Vulkan_Device_h

#include <Vortex/Renderer/BindGroup.h>
#include <Vortex/Renderer/CommandBuffer.h>
#include <Vortex/Renderer/Common.h>
#include <Vortex/Renderer/Device.h>
#include <Vortex/Renderer/Pipeline.h>
#include <map>

#include "Instance.h"
#include "WebGPU.h"

namespace Vortex
{
namespace Renderer
{
class WebGPUDevice : public Device
{
public:
  VORTEX_API WebGPUDevice(const Instance& instance);

  VORTEX_API ~WebGPUDevice();

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

  // WebGPU specific functiosn

  VORTEX_API WGPUDevice Handle() const;

  VORTEX_API WGPUQueue Queue() const;

private:
  WGPUDevice mDevice;
  WGPUQueue mQueue;

  std::unique_ptr<CommandBuffer> mExecute;
  std::vector<std::tuple<SPIRV::ShaderLayouts, WGPUBindGroupLayout>> mGroupLayouts;
  std::vector<std::tuple<SPIRV::ShaderLayouts, WGPUPipelineLayout>> mPipelineLayouts;
};

}  // namespace Renderer
}  // namespace Vortex

#endif
