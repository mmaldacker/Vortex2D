//
//  ConjugateGradient.cpp
//  Vertex2D
//

#include "ConjugateGradient.h"

namespace Vortex2D { namespace Fluid {

ConjugateGradient::ConjugateGradient(const Renderer::Device& device,
                                     const glm::ivec2& size,
                                     Preconditioner& preconditioner,
                                     bool statistics)
    : mPreconditioner(preconditioner)
    , r(device, vk::BufferUsageFlagBits::eStorageBuffer, false, size.x*size.y*sizeof(float))
    , s(device, vk::BufferUsageFlagBits::eStorageBuffer, false, size.x*size.y*sizeof(float))
    , z(device, vk::BufferUsageFlagBits::eStorageBuffer, false, size.x*size.y*sizeof(float))
    , inner(device, vk::BufferUsageFlagBits::eStorageBuffer, false, size.x*size.y*sizeof(float))
    , alpha(device, vk::BufferUsageFlagBits::eStorageBuffer, false, sizeof(float))
    , beta(device, vk::BufferUsageFlagBits::eStorageBuffer, false, sizeof(float))
    , rho(device, vk::BufferUsageFlagBits::eStorageBuffer, false, sizeof(float))
    , rho_new(device, vk::BufferUsageFlagBits::eStorageBuffer, false, sizeof(float))
    , sigma(device, vk::BufferUsageFlagBits::eStorageBuffer, false, sizeof(float))
    , error(device, vk::BufferUsageFlagBits::eStorageBuffer, false, sizeof(float))
    , errorLocal(device, vk::BufferUsageFlagBits::eStorageBuffer, true, sizeof(float))
    , matrixMultiply(device, size, "../Vortex2D/MultiplyMatrix.comp.spv",
                    {vk::DescriptorType::eStorageBuffer,
                     vk::DescriptorType::eStorageBuffer,
                     vk::DescriptorType::eStorageBuffer,
                     vk::DescriptorType::eStorageBuffer})
    , scalarDivision(device, glm::ivec2(1), "../Vortex2D/Divide.comp.spv",
                    {vk::DescriptorType::eStorageBuffer,
                     vk::DescriptorType::eStorageBuffer,
                     vk::DescriptorType::eStorageBuffer})
    , scalarMultiply(device, size, "../Vortex2D/Multiply.comp.spv",
                    {vk::DescriptorType::eStorageBuffer,
                     vk::DescriptorType::eStorageBuffer,
                     vk::DescriptorType::eStorageBuffer})
    , multiplyAdd(device, size, "../Vortex2D/MultiplyAdd.comp.spv",
                 {vk::DescriptorType::eStorageBuffer,
                  vk::DescriptorType::eStorageBuffer,
                  vk::DescriptorType::eStorageBuffer,
                  vk::DescriptorType::eStorageBuffer})
    , multiplySub(device, size, "../Vortex2D/MultiplySub.comp.spv",
                 {vk::DescriptorType::eStorageBuffer,
                  vk::DescriptorType::eStorageBuffer,
                  vk::DescriptorType::eStorageBuffer,
                  vk::DescriptorType::eStorageBuffer})
    , reduceSum(device, size)
    , reduceMax(device, size)
    , reduceMaxBound(reduceMax.Bind(r, error))
    , reduceSumRhoBound(reduceSum.Bind(inner, rho))
    , reduceSumSigmaBound(reduceSum.Bind(inner, sigma))
    , reduceSumRhoNewBound(reduceSum.Bind(inner, rho_new))
    , multiplyRBound(scalarMultiply.Bind({r, r, inner}))
    , multiplySBound(scalarMultiply.Bind({z, s, inner}))
    , multiplyZBound(scalarMultiply.Bind({z, r, inner}))
    , divideRhoBound(scalarDivision.Bind({rho, sigma, alpha}))
    , divideRhoNewBound(scalarDivision.Bind({rho_new, rho, beta}))
    , multiplySubRBound(multiplySub.Bind({r, z, alpha, r}))
    , multiplyAddSBound(multiplyAdd.Bind({r, s, beta, s}))
    , multiplyAddZBound(multiplyAdd.Bind({z, s, beta, s}))
    , mNormalSolveInit(device, false)
    , mNormalSolve(device, false)
    , mSolveInit(device, false)
    , mSolve(device, false)
    , mErrorRead(device)
    , mEnableStatistics(statistics)
    , mStatistics(device)
{

    mErrorRead.Record([&](vk::CommandBuffer commandBuffer)
    {
        errorLocal.CopyFrom(commandBuffer, error);
    });
}

void ConjugateGradient::Init(Renderer::Buffer& d,
                             Renderer::Buffer& l,
                             Renderer::Buffer& b,
                             Renderer::Buffer& pressure)
{
    mPreconditioner.Init(d, l, r, z);

    matrixMultiplyBound = matrixMultiply.Bind({d, l, s, z});
    multiplyAddPBound = multiplyAdd.Bind({pressure, s, alpha, pressure});

    mNormalSolveInit.Record([&](vk::CommandBuffer commandBuffer)
    {
        // r = b
        r.CopyFrom(commandBuffer, b);

        // calculate error
        reduceMaxBound.Record(commandBuffer);
        error.Barrier(commandBuffer, vk::AccessFlagBits::eShaderWrite, vk::AccessFlagBits::eShaderRead);

        // p = 0
        pressure.Clear(commandBuffer);

        // s = r
        s.CopyFrom(commandBuffer, r);

        // rho = rTr
        multiplyRBound.Record(commandBuffer);
        inner.Barrier(commandBuffer, vk::AccessFlagBits::eShaderWrite, vk::AccessFlagBits::eShaderRead);
        reduceSumRhoBound.Record(commandBuffer);
        rho.Barrier(commandBuffer, vk::AccessFlagBits::eShaderWrite, vk::AccessFlagBits::eShaderRead);
    });

    mNormalSolve.Record([&](vk::CommandBuffer commandBuffer)
    {
        // z = As
        matrixMultiplyBound.Record(commandBuffer);
        z.Barrier(commandBuffer, vk::AccessFlagBits::eShaderWrite, vk::AccessFlagBits::eShaderRead);

        // sigma = zTs
        multiplySBound.Record(commandBuffer);
        inner.Barrier(commandBuffer, vk::AccessFlagBits::eShaderWrite, vk::AccessFlagBits::eShaderRead);
        reduceSumSigmaBound.Record(commandBuffer);
        sigma.Barrier(commandBuffer, vk::AccessFlagBits::eShaderWrite, vk::AccessFlagBits::eShaderRead);

        // alpha = rho / sigma
        divideRhoBound.Record(commandBuffer);
        alpha.Barrier(commandBuffer, vk::AccessFlagBits::eShaderWrite, vk::AccessFlagBits::eShaderRead);

        // p = p + alpha * s
        multiplyAddPBound.Record(commandBuffer);
        pressure.Barrier(commandBuffer, vk::AccessFlagBits::eShaderWrite, vk::AccessFlagBits::eShaderRead);

        // r = r - alpha * z
        multiplySubRBound.Record(commandBuffer);
        r.Barrier(commandBuffer, vk::AccessFlagBits::eShaderWrite, vk::AccessFlagBits::eShaderRead);

        // calculate max error
        reduceMaxBound.Record(commandBuffer);
        error.Barrier(commandBuffer, vk::AccessFlagBits::eShaderWrite, vk::AccessFlagBits::eShaderRead);

        // rho_new = rTr
        multiplyRBound.Record(commandBuffer);
        inner.Barrier(commandBuffer, vk::AccessFlagBits::eShaderWrite, vk::AccessFlagBits::eShaderRead);
        reduceSumRhoNewBound.Record(commandBuffer);
        rho_new.Barrier(commandBuffer, vk::AccessFlagBits::eShaderWrite, vk::AccessFlagBits::eShaderRead);

        // beta = rho_new / rho
        divideRhoNewBound.Record(commandBuffer);
        beta.Barrier(commandBuffer, vk::AccessFlagBits::eShaderWrite, vk::AccessFlagBits::eShaderRead);

        // s = r + beta * s
        multiplyAddSBound.Record(commandBuffer);
        s.Barrier(commandBuffer, vk::AccessFlagBits::eShaderWrite, vk::AccessFlagBits::eShaderRead);

        // rho = rho_new
        rho.CopyFrom(commandBuffer, rho_new);
    });

    mSolveInit.Record([&](vk::CommandBuffer commandBuffer)
    {
        // r = b
        r.CopyFrom(commandBuffer, b);

        // calculate error
        reduceMaxBound.Record(commandBuffer);
        error.Barrier(commandBuffer, vk::AccessFlagBits::eShaderWrite, vk::AccessFlagBits::eShaderRead);

        // p = 0
        pressure.Clear(commandBuffer);

        // z = M^-1 r
        z.Clear(commandBuffer);
        mPreconditioner.Record(commandBuffer);
        z.Barrier(commandBuffer, vk::AccessFlagBits::eShaderWrite, vk::AccessFlagBits::eShaderRead);

        // s = z
        s.CopyFrom(commandBuffer, z);

        // rho = zTr
        multiplyZBound.Record(commandBuffer);
        inner.Barrier(commandBuffer, vk::AccessFlagBits::eShaderWrite, vk::AccessFlagBits::eShaderRead);
        reduceSumRhoBound.Record(commandBuffer);
        rho.Barrier(commandBuffer, vk::AccessFlagBits::eShaderWrite, vk::AccessFlagBits::eShaderRead);
    });

    mSolve.Record([&](vk::CommandBuffer commandBuffer)
    {
        if (mEnableStatistics) mStatistics.Start(commandBuffer);

        // z = As
        matrixMultiplyBound.Record(commandBuffer);
        z.Barrier(commandBuffer, vk::AccessFlagBits::eShaderWrite, vk::AccessFlagBits::eShaderRead);
        if (mEnableStatistics) mStatistics.Tick(commandBuffer, "z=As");

        // sigma = zTs
        multiplySBound.Record(commandBuffer);
        inner.Barrier(commandBuffer, vk::AccessFlagBits::eShaderWrite, vk::AccessFlagBits::eShaderRead);
        reduceSumSigmaBound.Record(commandBuffer);
        if (mEnableStatistics) mStatistics.Tick(commandBuffer, "inner=z*s");
        sigma.Barrier(commandBuffer, vk::AccessFlagBits::eShaderWrite, vk::AccessFlagBits::eShaderRead);
        if (mEnableStatistics) mStatistics.Tick(commandBuffer, "sigma=zTs");

        // alpha = rho / sigma
        divideRhoBound.Record(commandBuffer);
        alpha.Barrier(commandBuffer, vk::AccessFlagBits::eShaderWrite, vk::AccessFlagBits::eShaderRead);
        if (mEnableStatistics) mStatistics.Tick(commandBuffer, "alpha=rho/sigma");

        // p = p + alpha * s
        multiplyAddPBound.Record(commandBuffer);
        pressure.Barrier(commandBuffer, vk::AccessFlagBits::eShaderWrite, vk::AccessFlagBits::eShaderRead);
        if (mEnableStatistics) mStatistics.Tick(commandBuffer, "p=p+alpha*s");

        // r = r - alpha * z
        multiplySubRBound.Record(commandBuffer);
        r.Barrier(commandBuffer, vk::AccessFlagBits::eShaderWrite, vk::AccessFlagBits::eShaderRead);
        if (mEnableStatistics) mStatistics.Tick(commandBuffer, "r=r-alpha*z");

        // calculate max error
        reduceMaxBound.Record(commandBuffer);
        error.Barrier(commandBuffer, vk::AccessFlagBits::eShaderWrite, vk::AccessFlagBits::eShaderRead);
        if (mEnableStatistics) mStatistics.Tick(commandBuffer, "error=max(r)");

        // z = M^-1 r
        z.Clear(commandBuffer);
        mPreconditioner.Record(commandBuffer);
        z.Barrier(commandBuffer, vk::AccessFlagBits::eShaderWrite, vk::AccessFlagBits::eShaderRead);
        if (mEnableStatistics) mStatistics.Tick(commandBuffer, "z=M^-1r");

        // rho_new = zTr
        multiplyZBound.Record(commandBuffer);
        inner.Barrier(commandBuffer, vk::AccessFlagBits::eShaderWrite, vk::AccessFlagBits::eShaderRead);
        if (mEnableStatistics) mStatistics.Tick(commandBuffer, "inner=z*r");
        reduceSumRhoNewBound.Record(commandBuffer);
        rho_new.Barrier(commandBuffer, vk::AccessFlagBits::eShaderWrite, vk::AccessFlagBits::eShaderRead);
        if (mEnableStatistics) mStatistics.Tick(commandBuffer, "rho_new=zTr");

        // beta = rho_new / rho
        divideRhoNewBound.Record(commandBuffer);
        beta.Barrier(commandBuffer, vk::AccessFlagBits::eShaderWrite, vk::AccessFlagBits::eShaderRead);
        if (mEnableStatistics) mStatistics.Tick(commandBuffer, "beta=rho_new/rho");

        // s = z + beta * s
        multiplyAddZBound.Record(commandBuffer);
        s.Barrier(commandBuffer, vk::AccessFlagBits::eShaderWrite, vk::AccessFlagBits::eShaderRead);
        if (mEnableStatistics) mStatistics.Tick(commandBuffer, "s=z+beta*s");

        // rho = rho_new
        rho.CopyFrom(commandBuffer, rho_new);
        if (mEnableStatistics) mStatistics.End(commandBuffer, "rho=rho_new");
    });
}

void ConjugateGradient::Solve(Parameters& params)
{
    mSolveInit.Submit();
    mErrorRead.Submit();

    for (unsigned i = 0;; ++i)
    {
        // exit condition
        mErrorRead.Wait();

        params.OutIterations = i;
        errorLocal.CopyTo(params.OutError);
        if (params.IsFinished(i, params.OutError))
        {
            return;
        }

        mErrorRead.Submit();
        mSolve.Submit();
    }
}

void ConjugateGradient::NormalSolve(Parameters& params)
{
    mNormalSolveInit.Submit();
    mErrorRead.Submit();

    for (unsigned i = 0 ;; ++i)
    {
        // exit condition
        mErrorRead.Wait();

        params.OutIterations = i;
        errorLocal.CopyTo(params.OutError);
        if (params.IsFinished(i, params.OutError))
        {
            return;
        }

        mErrorRead.Submit();
        mNormalSolve.Submit();
    }
}

Renderer::Statistics::Timestamps ConjugateGradient::GetStatistics()
{
    return mStatistics.GetTimestamps();
}

}}
