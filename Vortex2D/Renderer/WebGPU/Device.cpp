//
//  Device.cpp
//  Vortex2D
//

#include <fstream>
#include <iostream>

#include "Device.h"

namespace Vortex2D
{
namespace Renderer
{
WebGPUDevice::WebGPUDevice(const Instance& instance) : mDevice(0), mQueue(0)
{
  mDevice = wgpu_adapter_request_device(instance.GetAdapter(), nullptr, nullptr);
  if (mDevice == 0)
  {
    throw std::runtime_error("Error creating device");
  }

  mQueue = wgpu_device_get_default_queue(mDevice);
  if (mQueue == 0)
  {
    throw std::runtime_error("Invalid queue");
  }

  mExecute = std::make_unique<CommandBuffer>(*this, true);
}

WebGPUDevice::~WebGPUDevice()
{
  if (mDevice != 0)
  {
    wgpu_device_destroy(mDevice);
  }
}

bool WebGPUDevice::HasTimer() const
{
  return false;
}

void WebGPUDevice::WaitIdle()
{
  wgpu_device_poll(mDevice, true);
}

void WebGPUDevice::Execute(CommandBuffer::CommandFn commandFn) const
{
  mExecute->Record(commandFn);
  mExecute->Submit();
  mExecute->Wait();
}

Handle::ShaderModule WebGPUDevice::CreateShaderModule(const SpirvBinary& spirv)
{
  // TODO missing caching

  WGPUShaderModuleDescriptor descriptor{};
  descriptor.code = {spirv.data(), spirv.words()};

  return reinterpret_cast<Handle::ShaderModule>(
      wgpu_device_create_shader_module(mDevice, &descriptor));
}

Handle::BindGroupLayout WebGPUDevice::CreateBindGroupLayout(const SPIRV::ShaderLayouts& layouts)
{
  auto it = std::find_if(
      mGroupLayouts.begin(), mGroupLayouts.end(), [&](const auto& descriptorSetLayout) {
        return std::get<0>(descriptorSetLayout) == layouts;
      });

  if (it == mGroupLayouts.end())
  {
    std::vector<WGPUBindGroupLayoutEntry> entries;
    for (auto& layout : layouts)
    {
      for (auto& desciptorType : layout.bindings)
      {
        WGPUBindGroupLayoutEntry entry{};
        entry.binding = desciptorType.first;
        entry.visibility = ConvertShaderStage(layout.shaderStage);
        entry.ty = ConvertBindingType(desciptorType.second);

        // TODO do we need to fill the rest of the struct?

        entries.emplace_back(entry);
      }
    }

    WGPUBindGroupLayoutDescriptor descriptor{};
    descriptor.entries = entries.data();
    descriptor.entries_length = entries.size();

    auto handle = wgpu_device_create_bind_group_layout(mDevice, &descriptor);

    mGroupLayouts.emplace_back(layouts, handle);
    return reinterpret_cast<Handle::BindGroupLayout>(handle);
  }

  auto handle = std::get<1>(*it);
  return reinterpret_cast<Handle::BindGroupLayout>(handle);
}

Handle::PipelineLayout WebGPUDevice::CreatePipelineLayout(const SPIRV::ShaderLayouts& layouts)
{
  auto it = std::find_if(
      mPipelineLayouts.begin(), mPipelineLayouts.end(), [&](const auto& pipelineLayout) {
        return std::get<0>(pipelineLayout) == layouts;
      });

  if (it == mPipelineLayouts.end())
  {
    WGPUBindGroupLayoutId bindGroupLayouts[] = {
        Handle::ConvertBindGroupLayout(CreateBindGroupLayout(layouts))};

    WGPUPipelineLayoutDescriptor descriptor{};
    descriptor.bind_group_layouts = bindGroupLayouts;
    descriptor.bind_group_layouts_length = 1;

    auto handle = wgpu_device_create_pipeline_layout(mDevice, &descriptor);

    mPipelineLayouts.emplace_back(layouts, handle);
    return reinterpret_cast<Handle::PipelineLayout>(handle);
  }

  auto handle = std::get<1>(*it);
  return reinterpret_cast<Handle::PipelineLayout>(handle);
}

BindGroup WebGPUDevice::CreateBindGroup(const Handle::BindGroupLayout& bindGroupLayout,
                                        const SPIRV::ShaderLayouts& layouts,
                                        const std::vector<BindingInput>& bindingInputs)
{
  std::vector<WGPUBindGroupEntry> entries;
  for (std::size_t i = 0; i < bindingInputs.size(); i++)
  {
    WGPUBindGroupEntry entry{};
    entry.binding = bindingInputs[i].Bind == BindingInput::DefaultBind ? static_cast<uint32_t>(i)
                                                                       : bindingInputs[i].Bind;

    bindingInputs[i].Input.match(
        [&](Renderer::GenericBuffer* buffer) {
          WGPUBufferBinding binding{};
          binding.buffer = reinterpret_cast<WGPUBufferId>(buffer->Handle());
          binding.offset = 0;
          binding.size = buffer->Size();

          WGPUBindingResource resource{};
          resource.tag = WGPUBindingResource_Buffer;
          resource.buffer = {binding};

          entry.resource = resource;
        },
        [&](Image image) {
          WGPUBindingResource resource{};

          if (image.Sampler != nullptr)
          {
            resource.tag = WGPUBindingResource_Sampler;
            resource.sampler = {reinterpret_cast<WGPUSamplerId>(image.Sampler->Handle())};
          }
          else
          {
            resource.tag = WGPUBindingResource_TextureView;
            resource.texture_view = {reinterpret_cast<WGPUTextureViewId>(image.Texture->GetView())};
          }

          entry.resource = resource;
        });

    entries.emplace_back(entry);
  }

  WGPUBindGroupDescriptor descriptor{};
  descriptor.layout = Handle::ConvertBindGroupLayout(bindGroupLayout);
  descriptor.entries = entries.data();
  descriptor.entries_length = entries.size();

  return {reinterpret_cast<Handle::BindGroup>(wgpu_device_create_bind_group(mDevice, &descriptor))};
}

Handle::Pipeline WebGPUDevice::CreateGraphicsPipeline(const GraphicsPipelineDescriptor& graphics,
                                                      const RenderState& renderState)
{
  return {};
}

Handle::Pipeline WebGPUDevice::CreateComputePipeline(Handle::ShaderModule shader,
                                                     Handle::PipelineLayout layout,
                                                     SpecConstInfo specConstInfo)
{
  // TODO missing caching

  WGPUProgrammableStageDescriptor computeStage{};
  computeStage.entry_point = "main";
  computeStage.module = reinterpret_cast<WGPUShaderModuleId>(shader);

  WGPUComputePipelineDescriptor descriptor{};
  descriptor.compute_stage = computeStage;
  descriptor.layout = reinterpret_cast<WGPUPipelineLayoutId>(layout);

  return reinterpret_cast<Handle::Pipeline>(
      wgpu_device_create_compute_pipeline(mDevice, &descriptor));
}

WGPUDeviceId WebGPUDevice::Handle() const
{
  return mDevice;
}

WGPUQueueId WebGPUDevice::Queue() const
{
  return mQueue;
}

}  // namespace Renderer
}  // namespace Vortex2D
