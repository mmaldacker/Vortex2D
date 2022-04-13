//
//  Work.cpp
//  Vortex
//

#include "Work.h"

#include <Vortex/Renderer/BindGroup.h>
#include <Vortex/SPIRV/Reflection.h>

namespace Vortex
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
  auto localSize = ComputeSize::GetLocalSize2D();
  localSize.x /= 2;
  localSize.y *= 2;

  ComputeSize computeSize(ComputeSize::Default2D());
  computeSize.DomainSize = size;
  computeSize.LocalSize = localSize;
  computeSize.WorkSize =
      glm::ceil(glm::vec2(size) / (glm::vec2(localSize) * glm::vec2(2.0f, 1.0f)));

  return computeSize;
}

DispatchParams::DispatchParams(int count)
    : workSize(static_cast<uint32_t>(
                   std::ceil(static_cast<float>(count) / ComputeSize::GetLocalSize1D())),
               1,
               1)
    , count(count)
{
}

Work::Work(Device& device,
           const ComputeSize& computeSize,
           const SpirvBinary& spirv,
           const SpecConstInfo& additionalSpecConstInfo)
    : mComputeSize(computeSize), mDevice(device)
{
  Handle::ShaderModule shaderModule = mDevice.CreateShaderModule(spirv);
  SPIRV::Reflection reflection(spirv);
  if (reflection.GetShaderStage() != ShaderStage::Compute)
    throw std::runtime_error("only compute supported");

  mLayout = {reflection};

  auto layout = mDevice.CreatePipelineLayout(mLayout);

  SpecConstInfo specConstInfo = additionalSpecConstInfo;

  assert(mComputeSize.LocalSize.x > 0 && mComputeSize.LocalSize.y > 0);
  if (mComputeSize.LocalSize.y != 1)
  {
    Detail::InsertSpecConst(specConstInfo,
                            SpecConstValue(1, mComputeSize.LocalSize.x),
                            SpecConstValue(2, mComputeSize.LocalSize.y));

    mPipeline = mDevice.CreateComputePipeline(shaderModule, layout, specConstInfo);
  }
  else
  {
    Detail::InsertSpecConst(specConstInfo, SpecConstValue(1, mComputeSize.LocalSize.x));

    mPipeline = mDevice.CreateComputePipeline(shaderModule, layout, specConstInfo);
  }
}

Work::Bound Work::Bind(ComputeSize computeSize, const std::vector<BindingInput>& inputs)
{
  if (inputs.size() != mLayout.front().bindings.size())
  {
    throw std::runtime_error("Unmatched inputs and bindings");
  }

  auto pipelineLayout = mDevice.CreatePipelineLayout(mLayout);
  auto bindGroupLayout = mDevice.CreateBindGroupLayout(mLayout);
  auto bindGroup = mDevice.CreateBindGroup(bindGroupLayout, mLayout, inputs);

  return Bound(computeSize,
               mLayout.front().pushConstantSize,
               pipelineLayout,
               mPipeline,
               std::move(bindGroup));
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
                   Handle::PipelineLayout layout,
                   Handle::Pipeline pipeline,
                   BindGroup bindGroup)
    : mComputeSize(computeSize)
    , mPushConstantSize(pushConstantSize)
    , mLayout(layout)
    , mPipeline(pipeline)
    , mBindGroup(std::move(bindGroup))
{
}

void Work::Bound::Record(CommandEncoder& command)
{
  PushConstantOffset(command, 0, mComputeSize.DomainSize.x);
  if (mComputeSize.DomainSize.y != 1)
  {
    PushConstantOffset(command, 4, mComputeSize.DomainSize.y);
  }

  command.SetBindGroup(PipelineBindPoint::Compute, mLayout, mBindGroup);
  command.SetPipeline(PipelineBindPoint::Compute, mPipeline);

  command.Dispatch(mComputeSize.WorkSize.x, mComputeSize.WorkSize.y, 1);
}

void Work::Bound::RecordIndirect(CommandEncoder& command,
                                 IndirectBuffer<DispatchParams>& dispatchParams)
{
  PushConstantOffset(command, 0, mComputeSize.DomainSize.x);
  if (mComputeSize.DomainSize.y != 1)
  {
    PushConstantOffset(command, 4, mComputeSize.DomainSize.y);
  }

  command.SetBindGroup(PipelineBindPoint::Compute, mLayout, mBindGroup);
  command.SetPipeline(PipelineBindPoint::Compute, mPipeline);
  command.DispatchIndirect(dispatchParams);
}

}  // namespace Renderer
}  // namespace Vortex
