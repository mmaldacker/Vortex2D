//
//  Device.h
//  Vortex2D
//

#ifndef Vortex2d_Vulkan_Device_h
#define Vortex2d_Vulkan_Device_h

#include <Vortex2D/Renderer/BindGroup.h>
#include <Vortex2D/Renderer/CommandBuffer.h>
#include <Vortex2D/Renderer/Common.h>
#include <Vortex2D/Renderer/Device.h>
#include <Vortex2D/Renderer/Pipeline.h>
#include <map>

#include "Instance.h"
#include "WebGPU.h"

namespace Vortex2D
{
namespace Renderer
{
class WebGPUDevice : public Device
{
public:
  VORTEX2D_API WebGPUDevice(const Instance& instance);

  VORTEX2D_API ~WebGPUDevice();

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

  VORTEX2D_API WGPUDeviceId Handle() const;

  VORTEX2D_API WGPUQueueId Queue() const;

private:
  WGPUDeviceId mDevice;
  WGPUQueueId mQueue;

  std::unique_ptr<CommandBuffer> mExecute;
  std::vector<std::tuple<SPIRV::ShaderLayouts, WGPUBindGroupLayoutId>> mGroupLayouts;
  std::vector<std::tuple<SPIRV::ShaderLayouts, WGPUPipelineLayoutId>> mPipelineLayouts;
};

}  // namespace Renderer
}  // namespace Vortex2D

#endif
