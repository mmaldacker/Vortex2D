//
//  Device.cpp
//  Vortex2D
//

#include <fstream>
#include <iostream>

#include "Device.h"

namespace Vortex
{
namespace Renderer
{
void RequestDeviceCallback(WGPUDevice received, void* userdata)
{
  auto promise = reinterpret_cast<std::promise<WGPUDevice>*>(userdata);
  promise->set_value(received);
}

WebGPUDevice::WebGPUDevice(const Instance& instance) : mDevice(0), mQueue(0)
{
  std::promise<WGPUDevice> devicePromise;

  WGPUDeviceDescriptor descriptor{};
  wgpuAdapterRequestDevice(
      instance.GetAdapter(), &descriptor, RequestDeviceCallback, (void*)&devicePromise);

  mDevice = devicePromise.get_future().get();
  if (mDevice == 0)
  {
    throw std::runtime_error("Error creating device");
  }

  mQueue = wgpuDeviceGetQueue(mDevice);
  if (mQueue == 0)
  {
    throw std::runtime_error("Invalid queue");
  }

  mExecute = std::make_unique<CommandBuffer>(*this, true);
}

WebGPUDevice::~WebGPUDevice()
{
  // TODO do we need to destroy mDevice?
}

bool WebGPUDevice::HasTimer() const
{
  return false;
}

void WebGPUDevice::WaitIdle()
{
  wgpuDevicePoll(mDevice, true);
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

  auto* spirvDescriptor =
      (WGPUShaderModuleSPIRVDescriptor*)malloc(sizeof(WGPUShaderModuleSPIRVDescriptor));
  spirvDescriptor->chain.next = NULL;
  spirvDescriptor->chain.sType = WGPUSType_ShaderModuleSPIRVDescriptor;
  spirvDescriptor->code = spirv.data();
  spirvDescriptor->codeSize = spirv.words();
  descriptor.nextInChain = (const WGPUChainedStruct*)spirvDescriptor;
  descriptor.label = NULL;

  return reinterpret_cast<Handle::ShaderModule>(wgpuDeviceCreateShaderModule(mDevice, &descriptor));
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
      for (auto& descriptorType : layout.bindings)
      {
        WGPUBindGroupLayoutEntry entry{};
        entry.binding = descriptorType.first;
        entry.visibility = ConvertShaderStage(layout.shaderStage);

        switch (descriptorType.second)
        {
          case BindType::StorageBuffer:
            entry.buffer.type = WGPUBufferBindingType_Storage;
            break;
          case BindType::UniformBuffer:
            entry.buffer.type = WGPUBufferBindingType_Uniform;
            break;
          case BindType::ImageSampler:
            entry.sampler.type = WGPUSamplerBindingType_Filtering;
            break;
            // TODO FIXME
            // case BindType::StorageImage:
            //   entry.storageTexture.
          default:
            assert(false);
        }

        // TODO do we need to fill the rest of the struct?

        entries.emplace_back(entry);
      }
    }

    WGPUBindGroupLayoutDescriptor descriptor{};
    descriptor.entries = entries.data();
    descriptor.entryCount = entries.size();

    auto handle = wgpuDeviceCreateBindGroupLayout(mDevice, &descriptor);

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
    WGPUBindGroupLayout bindGroupLayouts[] = {
        Handle::ConvertBindGroupLayout(CreateBindGroupLayout(layouts))};

    WGPUPipelineLayoutDescriptor descriptor{};
    descriptor.bindGroupLayouts = bindGroupLayouts;
    descriptor.bindGroupLayoutCount = 1;

    auto handle = wgpuDeviceCreatePipelineLayout(mDevice, &descriptor);

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
          entry.buffer = reinterpret_cast<WGPUBuffer>(buffer->Handle());
          entry.offset = 0;
          entry.size = buffer->Size();
        },
        [&](Image image) {
          if (image.Sampler != nullptr)
          {
            entry.sampler = {reinterpret_cast<WGPUSampler>(image.Sampler->Handle())};
          }
          else
          {
            entry.textureView = {reinterpret_cast<WGPUTextureView>(image.Texture->GetView())};
          }
        });

    entries.emplace_back(entry);
  }

  WGPUBindGroupDescriptor descriptor{};
  descriptor.layout = Handle::ConvertBindGroupLayout(bindGroupLayout);
  descriptor.entries = entries.data();
  descriptor.entryCount = entries.size();

  return {reinterpret_cast<Handle::BindGroup>(wgpuDeviceCreateBindGroup(mDevice, &descriptor))};
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
  computeStage.entryPoint = "main";
  computeStage.module = reinterpret_cast<WGPUShaderModule>(shader);

  WGPUComputePipelineDescriptor descriptor{};
  descriptor.computeStage = computeStage;
  descriptor.layout = reinterpret_cast<WGPUPipelineLayout>(layout);

  return reinterpret_cast<Handle::Pipeline>(wgpuDeviceCreateComputePipeline(mDevice, &descriptor));
}

WGPUDevice WebGPUDevice::Handle() const
{
  return mDevice;
}

WGPUQueue WebGPUDevice::Queue() const
{
  return mQueue;
}

}  // namespace Renderer
}  // namespace Vortex
