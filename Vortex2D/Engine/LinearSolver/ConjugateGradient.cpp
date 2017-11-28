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
    , r(device, size.x*size.y)
    , s(device, size.x*size.y)
    , z(device, size.x*size.y)
    , inner(device, size.x*size.y)
    , alpha(device, 1)
    , beta(device, 1)
    , rho(device, 1)
    , rho_new(device, 1)
    , sigma(device, 1)
    , error(device)
    , matrixMultiply(device, size, "../Vortex2D/MultiplyMatrix.comp.spv")
    , scalarDivision(device, glm::ivec2(1), "../Vortex2D/Divide.comp.spv")
    , scalarMultiply(device, size, "../Vortex2D/Multiply.comp.spv")
    , multiplyAdd(device, size, "../Vortex2D/MultiplyAdd.comp.spv")
    , multiplySub(device, size, "../Vortex2D/MultiplySub.comp.spv")
    , reduceSum(device, size)
    , reduceMax(device, size)
    , reduceMaxBound(reduceMax.Bind(r, error))
    , reduceSumRhoBound(reduceSum.Bind(inner, rho))
    , reduceSumSigmaBound(reduceSum.Bind(inner, sigma))
    , reduceSumRhoNewBound(reduceSum.Bind(inner, rho_new))
    , multiplySBound(scalarMultiply.Bind({z, s, inner}))
    , multiplyZBound(scalarMultiply.Bind({z, r, inner}))
    , divideRhoBound(scalarDivision.Bind({rho, sigma, alpha}))
    , divideRhoNewBound(scalarDivision.Bind({rho_new, rho, beta}))
    , multiplySubRBound(multiplySub.Bind({r, z, alpha, r}))
    , multiplyAddZBound(multiplyAdd.Bind({z, s, beta, s}))
    , mSolveInit(device, false)
    , mSolve(device, false)
    , mErrorRead(device)
    , mEnableStatistics(statistics)
    , mStatistics(device)
{

    mErrorRead.Record([&](vk::CommandBuffer commandBuffer)
    {
        error.Download(commandBuffer);
    });
}

void ConjugateGradient::Init(Renderer::GenericBuffer& d,
                             Renderer::GenericBuffer& l,
                             Renderer::GenericBuffer& b,
                             Renderer::GenericBuffer& pressure)
{
    mPreconditioner.Init(d, l, r, z);

    matrixMultiplyBound = matrixMultiply.Bind({d, l, s, z});
    multiplyAddPBound = multiplyAdd.Bind({pressure, s, alpha, pressure});

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
        Renderer::CopyTo(error, params.OutError);
        // TODO should divide by the initial error
        if (params.IsFinished(i, params.OutError))
        {
            return;
        }

        mErrorRead.Submit();
        mSolve.Submit();
    }
}

Renderer::Statistics::Timestamps ConjugateGradient::GetStatistics()
{
    return mStatistics.GetTimestamps();
}

}}
