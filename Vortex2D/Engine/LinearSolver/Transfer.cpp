//
//  Transfer.cpp
//  Vortex2D
//

#include "Transfer.h"

namespace Vortex2D { namespace Fluid {

Transfer::Transfer(const Renderer::Device& device)
    : mDevice(device)
    , mProlongateWork(device, glm::ivec2(2), "../Vortex2D/Prolongate.comp.spv",
                      {vk::DescriptorType::eStorageBuffer,
                       vk::DescriptorType::eStorageBuffer})
    , mRestrictWork(device, glm::ivec2(2), "../Vortex2D/Restrict.comp.spv",
                    {vk::DescriptorType::eStorageBuffer,
                     vk::DescriptorType::eStorageBuffer})
{

}

void Transfer::InitProlongate(const glm::ivec2& fineSize, Renderer::Buffer& fine, Renderer::Buffer& coarse)
{
    mProlongateBound.push_back(mProlongateWork.Bind(fineSize, {coarse, fine}));
    mProlongateBuffer.push_back(&fine);
}

void Transfer::InitRestrict(const glm::ivec2& fineSize, Renderer::Buffer& fine, Renderer::Buffer& coarse)
{
    glm::ivec2 coarseSize =  glm::ivec2(1) + fineSize / glm::ivec2(2);

    mRestrictBound.push_back(mRestrictWork.Bind(coarseSize, {fine, coarse}));
    mRestrictBuffer.push_back(&coarse);
}

void Transfer::Prolongate(vk::CommandBuffer commandBuffer, int level)
{
    assert(level < mProlongateBound.size());

    mProlongateBound[level].Record(commandBuffer);
    mProlongateBuffer[level]->Barrier(commandBuffer, vk::AccessFlagBits::eShaderWrite, vk::AccessFlagBits::eShaderRead);
}

void Transfer::Restrict(vk::CommandBuffer commandBuffer, int level)
{
    assert(level < mRestrictBound.size());

    mRestrictBound[level].Record(commandBuffer);
    mRestrictBuffer[level]->Barrier(commandBuffer, vk::AccessFlagBits::eShaderWrite, vk::AccessFlagBits::eShaderRead);
}

}}
