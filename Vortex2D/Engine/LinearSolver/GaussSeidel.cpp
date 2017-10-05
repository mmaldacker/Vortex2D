//
//  GaussSeidel.cpp
//  Vortex2D
//

#include "GaussSeidel.h"
#include <cmath>
#include <glm/gtc/constants.hpp>

namespace Vortex2D { namespace Fluid {


GaussSeidel::GaussSeidel(const Renderer::Device& device, const glm::ivec2& size)
    : mW(2.0f/(1.0f+std::sin(glm::pi<float>()/std::sqrt((float)(size.x*size.y)))))
    , mPreconditionerIterations(1)
    , mResidual(device, vk::BufferUsageFlagBits::eStorageBuffer, false, size.x*size.y*sizeof(float))
    , mError(device,  vk::BufferUsageFlagBits::eStorageBuffer, false, sizeof(float))
    , mErrorLocal(device, vk::BufferUsageFlagBits::eStorageBuffer, true, sizeof(float))
    , mGaussSeidel(device, Renderer::MakeCheckerboardComputeSize(size), "../Vortex2D/GaussSeidel.comp.spv")
    , mResidualWork(device, size, "../Vortex2D/Residual.comp.spv")
    , mReduceMax(device, size)
    , mReduceMaxBound(mReduceMax.Bind(mResidual, mError))
    , mGaussSeidelCmd(device, false)
    , mInitCmd(device, false)
    , mErrorCmd(device)
{
}

void GaussSeidel::SetW(float w)
{
    mW = w;
}

void GaussSeidel::SetPreconditionerIterations(int iterations)
{
    mPreconditionerIterations = iterations;
}

void GaussSeidel::Init(Renderer::Buffer& d,
                       Renderer::Buffer& l,
                       Renderer::Buffer& div,
                       Renderer::Buffer& pressure)
{
    mPressure = &pressure;

    mGaussSeidelBound = mGaussSeidel.Bind({pressure, d, l, div});
    mGaussSeidelCmd.Record([&](vk::CommandBuffer commandBuffer)
    {
        Record(commandBuffer, 1);
    });

    mInitCmd.Record([&](vk::CommandBuffer commandBuffer)
    {
        pressure.Clear(commandBuffer);
    });

    mResidualBound = mResidualWork.Bind({pressure, d, l, div, mResidual});

    mErrorCmd.Record([&](vk::CommandBuffer commandBuffer)
    {
        mResidualBound.Record(commandBuffer);
        mResidual.Barrier(commandBuffer, vk::AccessFlagBits::eShaderWrite, vk::AccessFlagBits::eShaderRead);

        mReduceMaxBound.Record(commandBuffer);
        mError.Barrier(commandBuffer, vk::AccessFlagBits::eShaderWrite, vk::AccessFlagBits::eShaderRead);

        mErrorLocal.CopyFrom(commandBuffer, mError);
    });
}

void GaussSeidel::Solve(Parameters& params)
{
    mInitCmd.Submit();
    mErrorCmd.Submit();

    for (unsigned i  = 0;; ++i)
    {
        // exit condition
        mErrorCmd.Wait();

        params.OutIterations = i;
        mErrorLocal.CopyTo(params.OutError);
        if (params.IsFinished(i, params.OutError))
        {
            return;
        }

        mErrorCmd.Submit();
        mGaussSeidelCmd.Submit();
    }
}

void GaussSeidel::Record(vk::CommandBuffer commandBuffer)
{
    assert(mPressure != nullptr);
    Record(commandBuffer, mPreconditionerIterations);
}

void GaussSeidel::Record(vk::CommandBuffer commandBuffer, int iterations)
{
    for (unsigned i  = 0; i < iterations; ++i)
    {
        mGaussSeidelBound.PushConstant(commandBuffer, 8, mW);
        mGaussSeidelBound.PushConstant(commandBuffer, 12, 1);
        mGaussSeidelBound.Record(commandBuffer);
        mPressure->Barrier(commandBuffer, vk::AccessFlagBits::eShaderWrite, vk::AccessFlagBits::eShaderRead);
        mGaussSeidelBound.PushConstant(commandBuffer, 12, 0);
        mGaussSeidelBound.Record(commandBuffer);
        mPressure->Barrier(commandBuffer, vk::AccessFlagBits::eShaderWrite, vk::AccessFlagBits::eShaderRead);
    }
}

}}
