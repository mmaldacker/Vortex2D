//
//  Work.cpp
//  Vortex2D
//

#include "Work.h"

#include <Vortex2D/Renderer/DescriptorSet.h>
#include <Vortex2D/SPIRV/Reflection.h>

namespace Vortex2D
{
namespace Renderer
{
glm::ivec2 ComputeSize::GetLocalSize2D()
{
  return {64, 4};
}

int ComputeSize::GetLocalSize1D()
{
  return 256;
}

glm::ivec2 ComputeSize::GetWorkSize(const glm::ivec2& size, const glm::ivec2& localSize)
{
  return glm::ceil(glm::vec2(size) / glm::vec2(localSize));
}

glm::ivec2 ComputeSize::GetWorkSize(int size, int localSize)
{
  return glm::ceil(glm::vec2(size, 1) / glm::vec2(localSize, 1));
}

ComputeSize::ComputeSize(const glm::ivec2& size, const glm::ivec2& localSize)
    : DomainSize(size), WorkSize(GetWorkSize(size, localSize)), LocalSize(localSize)
{
}

ComputeSize::ComputeSize(int size, int localSize)
    : DomainSize({size, 1}), WorkSize(GetWorkSize(size, localSize)), LocalSize({localSize, 1})
{
}

ComputeSize ComputeSize::Default2D()
{
  return ComputeSize(glm::ivec2{1});
}

ComputeSize ComputeSize::Default1D()
{
  return ComputeSize(1);
}

ComputeSize MakeStencilComputeSize(const glm::ivec2& size, int radius)
{
  ComputeSize computeSize(ComputeSize::Default2D());

  auto localSize = ComputeSize::GetLocalSize2D();
  computeSize.DomainSize = size;
  computeSize.LocalSize = localSize;
  computeSize.WorkSize = glm::ceil(glm::vec2(size) / glm::vec2(localSize - glm::ivec2(2 * radius)));

  return computeSize;
}

ComputeSize MakeCheckerboardComputeSize(const glm::ivec2& size)
{
  auto localSize = ComputeSize::GetLocalSize2D() * glm::ivec2(1, 2);

  ComputeSize computeSize(ComputeSize::Default2D());
  computeSize.DomainSize = size;
  computeSize.LocalSize = localSize;
  computeSize.WorkSize =
      glm::ceil(glm::vec2(size) / (glm::vec2(localSize) * glm::vec2(2.0f, 1.0f)));

  return computeSize;
}

DispatchParams::DispatchParams(int count)
    : workSize(static_cast<uint32_t>(
                   std::ceil(static_cast<float>(count) / Renderer::ComputeSize::GetLocalSize1D())),
               1,
               1)
    , count(count)
{
}

Work::Work(const Device& device,
           const ComputeSize& computeSize,
           const SpirvBinary& spirv,
           const SpecConstInfo& additionalSpecConstInfo)
    : mComputeSize(computeSize), mDevice(device)
{
  vk::ShaderModule shaderModule = device.GetShaderModule(spirv);
  SPIRV::Reflection reflection(spirv);
  if (reflection.GetShaderStage() != vk::ShaderStageFlagBits::eCompute)
    throw std::runtime_error("only compute supported");

  mPipelineLayout = {{reflection}};
  auto layout = device.GetLayoutManager().GetPipelineLayout(mPipelineLayout);

  SpecConstInfo specConstInfo = additionalSpecConstInfo;

  assert(mComputeSize.LocalSize.x > 0 && mComputeSize.LocalSize.y > 0);
  if (mComputeSize.LocalSize.y != 1)
  {
    Detail::InsertSpecConst(specConstInfo,
                            SpecConstValue(1, mComputeSize.LocalSize.x),
                            SpecConstValue(2, mComputeSize.LocalSize.y));

    mPipeline =
        device.GetPipelineCache().CreateComputePipeline(shaderModule, layout, specConstInfo);
  }
  else
  {
    Detail::InsertSpecConst(specConstInfo, SpecConstValue(1, mComputeSize.LocalSize.x));

    mPipeline =
        device.GetPipelineCache().CreateComputePipeline(shaderModule, layout, specConstInfo);
  }
}

Work::Bound Work::Bind(ComputeSize computeSize, const std::vector<Renderer::BindingInput>& inputs)
{
  if (inputs.size() != mPipelineLayout.layouts.front().bindings.size())
  {
    throw std::runtime_error("Unmatched inputs and bindings");
  }

  auto descriptorSet = mDevice.GetLayoutManager().MakeDescriptorSet(mPipelineLayout);
  Renderer::Bind(mDevice, descriptorSet, mPipelineLayout, inputs);

  return Bound(computeSize,
               mPipelineLayout.layouts.front().pushConstantSize,
               descriptorSet.pipelineLayout,
               mPipeline,
               std::move(descriptorSet.descriptorSet));
}

Work::Bound Work::Bind(const std::vector<BindingInput>& inputs)
{
  return Bind(mComputeSize, inputs);
}

Work::Bound::Bound() : mComputeSize(ComputeSize::Default2D()), mLayout(nullptr), mPipeline(nullptr)
{
}

Work::Bound::Bound(const ComputeSize& computeSize,
                   uint32_t pushConstantSize,
                   vk::PipelineLayout layout,
                   vk::Pipeline pipeline,
                   vk::UniqueDescriptorSet descriptor)
    : mComputeSize(computeSize)
    , mPushConstantSize(pushConstantSize)
    , mLayout(layout)
    , mPipeline(pipeline)
    , mDescriptor(std::move(descriptor))
{
}

void Work::Bound::Record(vk::CommandBuffer commandBuffer)
{
  PushConstantOffset(commandBuffer, 0, mComputeSize.DomainSize.x);
  if (mComputeSize.DomainSize.y != 1)
  {
    PushConstantOffset(commandBuffer, 4, mComputeSize.DomainSize.y);
  }

  commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eCompute, mLayout, 0, {*mDescriptor}, {});
  commandBuffer.bindPipeline(vk::PipelineBindPoint::eCompute, mPipeline);

  commandBuffer.dispatch(mComputeSize.WorkSize.x, mComputeSize.WorkSize.y, 1);
}

void Work::Bound::RecordIndirect(vk::CommandBuffer commandBuffer,
                                 IndirectBuffer<DispatchParams>& dispatchParams)
{
  PushConstantOffset(commandBuffer, 0, mComputeSize.DomainSize.x);
  if (mComputeSize.DomainSize.y != 1)
  {
    PushConstantOffset(commandBuffer, 4, mComputeSize.DomainSize.y);
  }
  commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eCompute, mLayout, 0, {*mDescriptor}, {});
  commandBuffer.bindPipeline(vk::PipelineBindPoint::eCompute, mPipeline);

  commandBuffer.dispatchIndirect(dispatchParams.Handle(), 0);
}

}  // namespace Renderer
}  // namespace Vortex2D
