//
//  GaussSeidel.cpp
//  Vortex2D
//

#include "GaussSeidel.h"
#include <cmath>
#include <glm/gtc/constants.hpp>

#include "vortex2d_generated_spirv.h"

namespace Vortex2D { namespace Fluid {


GaussSeidel::GaussSeidel(const Renderer::Device& device, const glm::ivec2& size)
    : mW(2.0f/(1.0f+std::sin(glm::pi<float>()/std::sqrt((float)(size.x*size.y)))))
    , mPreconditionerIterations(1)
    , mResidual(device, size.x*size.y)
    , mError(device)
    , mLocalError(device, 1, VMA_MEMORY_USAGE_GPU_TO_CPU)
    , mGaussSeidel(device, Renderer::MakeCheckerboardComputeSize(size), SPIRV::GaussSeidel_comp)
    , mResidualWork(device, size, SPIRV::Residual_comp)
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

void GaussSeidel::Bind(Renderer::GenericBuffer& d,
                       Renderer::GenericBuffer& l,
                       Renderer::GenericBuffer& div,
                       Renderer::GenericBuffer& pressure)
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

        mLocalError.CopyFrom(commandBuffer, mError);
    });
}

void GaussSeidel::BindRigidbody(float /*delta*/,
                                Renderer::GenericBuffer& /*d*/,
                                RigidBody& /*rigidBody*/)
{

}

void GaussSeidel::Solve(Parameters& params, const std::vector<RigidBody*>& /*rigidbodies*/)
{
    params.Reset();

    mInitCmd.Submit();

    if (params.Type == Parameters::SolverType::Iterative)
    {
        mErrorCmd.Submit().Wait();

        Renderer::CopyTo(mLocalError, params.OutError);
        if (params.OutError <= params.ErrorTolerance)
        {
            return;
        }

        mErrorCmd.Submit();
    }

    auto initialError = params.OutError;
    for (unsigned i = 0; !params.IsFinished(initialError); params.OutIterations = ++i)
    {
        mGaussSeidelCmd.Submit();

        if (params.Type == Parameters::SolverType::Iterative)
        {
            mErrorCmd.Wait();
            Renderer::CopyTo(mLocalError, params.OutError);
            mErrorCmd.Submit();
        }
    }
}

void GaussSeidel::Record(vk::CommandBuffer commandBuffer)
{
    assert(mPressure != nullptr);
    Record(commandBuffer, mPreconditionerIterations);
}

void GaussSeidel::Record(vk::CommandBuffer commandBuffer, int iterations)
{
    for (int i  = 0; i < iterations; ++i)
    {
        mGaussSeidelBound.PushConstant(commandBuffer, mW, 1);
        mGaussSeidelBound.Record(commandBuffer);
        mPressure->Barrier(commandBuffer, vk::AccessFlagBits::eShaderWrite, vk::AccessFlagBits::eShaderRead);
        mGaussSeidelBound.PushConstant(commandBuffer, mW, 0);
        mGaussSeidelBound.Record(commandBuffer);
        mPressure->Barrier(commandBuffer, vk::AccessFlagBits::eShaderWrite, vk::AccessFlagBits::eShaderRead);
    }
}

Renderer::ComputeSize MakeLocalSize(const glm::ivec2& size)
{
    Renderer::ComputeSize computeSize(size);
    computeSize.WorkSize = glm::ivec2(1);
    computeSize.LocalSize = glm::ivec2(16); // TODO shouldn't be hardcoded 16

    return computeSize;
}

LocalGaussSeidel::LocalGaussSeidel(const Renderer::Device& device, const glm::ivec2& size)
  : mLocalGaussSeidel(device, MakeLocalSize(size), SPIRV::LocalGaussSeidel_comp)
{
    // TODO check size is within local size
}

void LocalGaussSeidel::Bind(Renderer::GenericBuffer& d,
                  Renderer::GenericBuffer& l,
                  Renderer::GenericBuffer& div,
                  Renderer::GenericBuffer& pressure)
{
    mPressure = &pressure;
    mLocalGaussSeidelBound = mLocalGaussSeidel.Bind({pressure, d, l, div});
}

void LocalGaussSeidel::Record(vk::CommandBuffer commandBuffer)
{
    mLocalGaussSeidelBound.Record(commandBuffer);
    mPressure->Barrier(commandBuffer, vk::AccessFlagBits::eShaderWrite, vk::AccessFlagBits::eShaderRead);
}

}}
