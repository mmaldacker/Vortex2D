//
//  Transfer.cpp
//  Vortex2D
//

#include "Transfer.h"

#include "vortex2d_generated_spirv.h"

namespace Vortex2D
{
namespace Fluid
{
Transfer::Transfer(const Renderer::Device& device)
    : mDevice(device)
    , mProlongateWork(device, Renderer::ComputeSize::Default2D(), SPIRV::Prolongate_comp)
    , mRestrictWork(device, Renderer::ComputeSize::Default2D(), SPIRV::Restrict_comp)
{
}

void Transfer::ProlongateBind(std::size_t level,
                              const glm::ivec2& fineSize,
                              Renderer::GenericBuffer& fine,
                              Renderer::GenericBuffer& fineDiagonal,
                              Renderer::GenericBuffer& coarse,
                              Renderer::GenericBuffer& coarseDiagonal)
{
  if (mProlongateBound.size() < level + 1)
  {
    mProlongateBound.resize(level + 1);
    mProlongateBuffer.resize(level + 1);
  }

  mProlongateBound[level] =
      mProlongateWork.Bind(fineSize, {fineDiagonal, fine, coarseDiagonal, coarse});
  mProlongateBuffer[level] = &fine;
}

void Transfer::RestrictBind(std::size_t level,
                            const glm::ivec2& fineSize,
                            Renderer::GenericBuffer& fine,
                            Renderer::GenericBuffer& fineDiagonal,
                            Renderer::GenericBuffer& coarse,
                            Renderer::GenericBuffer& coarseDiagonal)
{
  if (mRestrictBound.size() < level + 1)
  {
    mRestrictBound.resize(level + 1);
    mRestrictBuffer.resize(level + 1);
  }

  glm::ivec2 coarseSize = fineSize / glm::ivec2(2);

  mRestrictBound[level] =
      mRestrictWork.Bind(coarseSize, {fineDiagonal, fine, coarseDiagonal, coarse});
  mRestrictBuffer[level] = &coarse;
}

void Transfer::Prolongate(vk::CommandBuffer commandBuffer, std::size_t level)
{
  assert(level < mProlongateBound.size());

  mProlongateBound[level].Record(commandBuffer);
  mProlongateBuffer[level]->Barrier(
      commandBuffer, vk::AccessFlagBits::eShaderWrite, vk::AccessFlagBits::eShaderRead);
}

void Transfer::Restrict(vk::CommandBuffer commandBuffer, std::size_t level)
{
  assert(level < mRestrictBound.size());

  mRestrictBound[level].Record(commandBuffer);
  mRestrictBuffer[level]->Barrier(
      commandBuffer, vk::AccessFlagBits::eShaderWrite, vk::AccessFlagBits::eShaderRead);
}

}  // namespace Fluid
}  // namespace Vortex2D
