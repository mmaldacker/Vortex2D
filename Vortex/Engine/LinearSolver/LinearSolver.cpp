//
//  LinearSolver.cpp
//  Vortex
//

#include "LinearSolver.h"

#include "vortex_generated_spirv.h"

namespace Vortex
{
namespace Fluid
{
LinearSolver::Parameters::Parameters(SolverType type, unsigned iterations, float errorTolerance)
    : Type(type)
    , Iterations(iterations)
    , ErrorTolerance(errorTolerance)
    , OutIterations(0)
    , OutError(0.0f)
{
}

LinearSolver::Data::Data(const Renderer::Device& device,
                         const glm::ivec2& size,
                         VmaMemoryUsage memoryUsage)
    : Diagonal(device, size.x * size.y, memoryUsage)
    , Lower(device, size.x * size.y, memoryUsage)
    , B(device, size.x * size.y, memoryUsage)
    , X(device, size.x * size.y, memoryUsage)
{
  device.Execute([&](vk::CommandBuffer commandBuffer) {
    Diagonal.Clear(commandBuffer);
    Lower.Clear(commandBuffer);
    B.Clear(commandBuffer);
    X.Clear(commandBuffer);
  });
}

LinearSolver::DebugData::DebugData(const Renderer::Device& device, const glm::ivec2& size)
    : Diagonal(device, size.x, size.y, vk::Format::eR32Sfloat)
    , Lower(device, size.x, size.y, vk::Format::eR32G32Sfloat)
    , B(device, size.x, size.y, vk::Format::eR32Sfloat)
    , X(device, size.x, size.y, vk::Format::eR32Sfloat)
{
}

LinearSolver::DebugCopy::DebugCopy(const Renderer::Device& device,
                                   const glm::ivec2& size,
                                   LinearSolver::Data& data,
                                   LinearSolver::DebugData& debugData)
    : mDebugDataCopy(device, size, SPIRV::DebugDataCopy_comp)
    , mDebugDataCopyBound(mDebugDataCopy.Bind({data.Diagonal,
                                               data.Lower,
                                               data.X,
                                               data.B,
                                               debugData.Diagonal,
                                               debugData.Lower,
                                               debugData.X,
                                               debugData.B}))
    , mCopy(device, false)
{
  mCopy.Record([&](vk::CommandBuffer commandBuffer) {
    commandBuffer.debugMarkerBeginEXT({"Debug data copy", {{0.30f, 0.01f, 0.19f, 1.0f}}},
                                      device.Loader());
    mDebugDataCopyBound.Record(commandBuffer);
    commandBuffer.debugMarkerEndEXT(device.Loader());
  });
}

void LinearSolver::DebugCopy::Copy()
{
  mCopy.Submit();
}

bool LinearSolver::Parameters::IsFinished(float initialError) const
{
  if (Type == SolverType::Fixed)
  {
    return OutIterations >= Iterations;
  }

  if (Iterations > 0)
  {
    return OutIterations >= Iterations || OutError <= ErrorTolerance * initialError;
  }
  else
  {
    return OutError <= ErrorTolerance;
  }
}

void LinearSolver::Parameters::Reset()
{
  OutError = 0.0f;
  OutIterations = 0;
}

LinearSolver::Parameters FixedParams(unsigned iterations)
{
  return LinearSolver::Parameters(LinearSolver::Parameters::SolverType::Fixed, iterations);
}

LinearSolver::Parameters IterativeParams(float errorTolerance)
{
  return LinearSolver::Parameters(
      LinearSolver::Parameters::SolverType::Iterative, 1000, errorTolerance);
}

LinearSolver::Error::Error(const Renderer::Device& device, const glm::ivec2& size)
    : mResidual(device, size.x * size.y)
    , mError(device)
    , mLocalError(device, 1, VMA_MEMORY_USAGE_GPU_TO_CPU)
    , mResidualWork(device, size, SPIRV::Residual_comp)
    , mReduceMax(device, size)
    , mReduceMaxBound(mReduceMax.Bind(mResidual, mError))
    , mErrorCmd(device)
{
  device.Execute([&](vk::CommandBuffer command) { mResidual.Clear(command); });
}

void LinearSolver::Error::Bind(Renderer::GenericBuffer& d,
                               Renderer::GenericBuffer& l,
                               Renderer::GenericBuffer& div,
                               Renderer::GenericBuffer& pressure)
{
  mResidualBound = mResidualWork.Bind({pressure, d, l, div, mResidual});

  mErrorCmd.Record([&](vk::CommandBuffer commandBuffer) {
    mResidualBound.Record(commandBuffer);
    mResidual.Barrier(
        commandBuffer, vk::AccessFlagBits::eShaderWrite, vk::AccessFlagBits::eShaderRead);

    mReduceMaxBound.Record(commandBuffer);
    mError.Barrier(
        commandBuffer, vk::AccessFlagBits::eShaderWrite, vk::AccessFlagBits::eShaderRead);

    mLocalError.CopyFrom(commandBuffer, mError);
  });
}

LinearSolver::Error& LinearSolver::Error::Submit()
{
  mErrorCmd.Submit();
  return *this;
}

LinearSolver::Error& LinearSolver::Error::Wait()
{
  mErrorCmd.Wait();
  return *this;
}

float LinearSolver::Error::GetError()
{
  float error;
  Renderer::CopyTo(mLocalError, error);
  return error;
}

}  // namespace Fluid
}  // namespace Vortex
