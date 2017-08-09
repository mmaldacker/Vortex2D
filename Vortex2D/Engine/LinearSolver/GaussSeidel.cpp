//
//  GaussSeidel.cpp
//  Vortex2D
//

#include "GaussSeidel.h"

#include <glm/gtc/constants.hpp>

namespace Vortex2D { namespace Fluid {


GaussSeidel::GaussSeidel(const Renderer::Device& device, const glm::ivec2& size)
    : mW(2.0f/(1.0f+std::sin(glm::pi<float>()/std::sqrt(size.x*size.y))))
    , mGaussSeidel(device, size, "../Vortex2D/GaussSeidel.comp.spv",
                   {vk::DescriptorType::eStorageBuffer,
                    vk::DescriptorType::eStorageBuffer,
                    vk::DescriptorType::eStorageBuffer},
                   8)
    , mGaussSeidelCmd(device, false)
    , mInitCmd(device, false)
{
}

void GaussSeidel::Init(Renderer::Buffer& matrix, Renderer::Buffer& div, Renderer::Buffer& pressure)
{
    mGaussSeidelBound = mGaussSeidel.Bind({pressure, matrix, div});
    mGaussSeidelCmd.Record([&](vk::CommandBuffer commandBuffer)
    {
        // TODO add barrier
        mGaussSeidelBound.PushConstant(commandBuffer, 8, mW);
        mGaussSeidelBound.PushConstant(commandBuffer, 12, 1);
        mGaussSeidelBound.Record(commandBuffer);
        pressure.Barrier(commandBuffer, vk::AccessFlagBits::eShaderWrite, vk::AccessFlagBits::eShaderRead);
        mGaussSeidelBound.PushConstant(commandBuffer, 12, 0);
        mGaussSeidelBound.Record(commandBuffer);
        pressure.Barrier(commandBuffer, vk::AccessFlagBits::eShaderWrite, vk::AccessFlagBits::eShaderRead);
    });

    mInitCmd.Record([&](vk::CommandBuffer commandBuffer)
    {
        pressure.Clear(commandBuffer);
    });
}

void GaussSeidel::Solve(Parameters& params)
{
    // FIXME implement solving within error tolerance
    assert(params.Iterations > 0);

    mInitCmd.Submit();

    for (unsigned i  = 0; !params.IsFinished(i, 0.0f); ++i)
    {
        mGaussSeidelCmd.Submit();
        params.OutIterations = i;
    }
}

}}
