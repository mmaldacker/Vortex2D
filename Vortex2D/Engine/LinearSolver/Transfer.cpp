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

void Transfer::Init(const glm::ivec2& fineSize, Renderer::Buffer& fine, Renderer::Buffer& coarse)
{
    glm::ivec2 coarseSize =  glm::ivec2(2) + (fineSize - glm::ivec2(2)) / glm::ivec2(2);

    mProlongateBound.push_back(mProlongateWork.Bind(fineSize, {coarse, fine}));
    auto& prolongateBound = mProlongateBound.back();

    mProlongateCmd.emplace_back(mDevice, false);
    mProlongateCmd.back().Record([&](vk::CommandBuffer commandBuffer)
    {
        prolongateBound.Record(commandBuffer);
        fine.Barrier(commandBuffer, vk::AccessFlagBits::eShaderWrite, vk::AccessFlagBits::eShaderRead);
    });

    mRestrictBound.push_back(mRestrictWork.Bind(coarseSize, {fine, coarse}));
    auto& restrictBound = mRestrictBound.back();

    mRestrictCmd.emplace_back(mDevice, false);
    mRestrictCmd.back().Record([&](vk::CommandBuffer commandBuffer)
    {
        restrictBound.Record(commandBuffer);
        coarse.Barrier(commandBuffer, vk::AccessFlagBits::eShaderWrite, vk::AccessFlagBits::eShaderRead);
    });
}

void Transfer::Prolongate(int level)
{
    assert(level < mProlongateCmd.size());
    mProlongateCmd[level].Submit();
}

void Transfer::Restrict(int level)
{
    assert(level < mRestrictCmd.size());
    mRestrictCmd[level].Submit();
}

}}
