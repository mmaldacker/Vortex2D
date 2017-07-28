//
//  GaussSeidel.cpp
//  Vortex2D
//

#include "GaussSeidel.h"

#include <glm/gtc/constants.hpp>

namespace Vortex2D { namespace Fluid {


GaussSeidel::GaussSeidel(const Renderer::Device& device, const glm::vec2& size)
    : mW(2.0f/(1.0f+std::sin(glm::pi<float>()/std::sqrt(size.x*size.y))))
    , mGaussSeidel(device, size, "../Vortex2D/GaussSeidel.comp.spv",
                   {vk::DescriptorType::eStorageBuffer,
                    vk::DescriptorType::eStorageBuffer},
                   8)
    , mGaussSeidelCmd(device, false)
{
}

void GaussSeidel::Init(Renderer::Buffer& data, Renderer::Buffer& pressure)
{
    mGaussSeidelBound = mGaussSeidel.Bind({pressure, data});
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
}

void GaussSeidel::Solve(Parameters& params)
{
    // FIXME implement solving within error tolerance
    assert(params.Iterations > 0);

    for (unsigned i  = 0; !params.IsFinished(i, 0.0f); ++i)
    {
        mGaussSeidelCmd.Submit();
        params.OutIterations = i;
    }
}

}}
