//
//  ConjugateGradient.cpp
//  Vertex2D
//

#include "ConjugateGradient.h"

namespace Vortex2D { namespace Fluid {

ConjugateGradient::ConjugateGradient(const Renderer::Device& device, const glm::ivec2& size)
    : r(device, vk::BufferUsageFlagBits::eStorageBuffer, false, size.x*size.y*sizeof(float))
    , s(device, vk::BufferUsageFlagBits::eStorageBuffer, false, size.x*size.y*sizeof(float))
    , alpha(device, vk::BufferUsageFlagBits::eStorageBuffer, false, sizeof(float))
    , beta(device, vk::BufferUsageFlagBits::eStorageBuffer, false, sizeof(float))
    , rho(device, vk::BufferUsageFlagBits::eStorageBuffer, false, sizeof(float))
    , rho_new(device, vk::BufferUsageFlagBits::eStorageBuffer, false, sizeof(float))
    , sigma(device, vk::BufferUsageFlagBits::eStorageBuffer, false, sizeof(float))
    , error(device, vk::BufferUsageFlagBits::eStorageBuffer, false, sizeof(float))
    , errorLocal(device, vk::BufferUsageFlagBits::eStorageBuffer, true, sizeof(float))
    , inner(device, vk::BufferUsageFlagBits::eStorageBuffer, false, size.x*size.y*sizeof(float))
    , z(device, vk::BufferUsageFlagBits::eStorageBuffer, false, size.x*size.y*sizeof(float))
    , matrixMultiply(device, size, "../Vortex2D/MultiplyMatrix.comp.spv", {vk::DescriptorType::eStorageBuffer,
                     vk::DescriptorType::eStorageBuffer,
                     vk::DescriptorType::eStorageBuffer})
    , scalarDivision(device, {1, 1}, "../Vortex2D/Divide.comp.spv", {vk::DescriptorType::eStorageBuffer,
                     vk::DescriptorType::eStorageBuffer,
                     vk::DescriptorType::eStorageBuffer})
    , scalarMultiply(device, {1, 1}, "../Vortex2D/Multiply.comp.spv", {vk::DescriptorType::eStorageBuffer,
                     vk::DescriptorType::eStorageBuffer,
                     vk::DescriptorType::eStorageBuffer})
    , multiplyAdd(device, size, "../Vortex2D/MultiplyAdd.comp.spv", {vk::DescriptorType::eStorageBuffer,
                  vk::DescriptorType::eStorageBuffer,
                  vk::DescriptorType::eStorageBuffer,
                  vk::DescriptorType::eStorageBuffer})
    , multiplySub(device, size, "../Vortex2D/MultiplySub.comp.spv", {vk::DescriptorType::eStorageBuffer,
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
    , multiplyZBound(scalarMultiply.Bind({z, s, inner}))
    , divideRhoBound(scalarDivision.Bind({rho, sigma, alpha}))
    , divideRhoNewBound(scalarDivision.Bind({rho_new, rho, beta}))
    , multiplySubRBound(multiplySub.Bind({r, z, alpha, r}))
    , multiplyAddSBound(multiplyAdd.Bind({r, s, beta, s}))
    , mNormalSolveInit(device, false)
    , mNormalSolve(device, false)
    , mErrorRead(device)
{

    mErrorRead.Record([&](vk::CommandBuffer commandBuffer)
    {
        errorLocal.CopyFrom(commandBuffer, error);
    });
}

void ConjugateGradient::Init(Renderer::Buffer& A,
                             Renderer::Buffer& b,
                             Renderer::Buffer& pressure,
                             Renderer::Work& buildMatrix,
                             Renderer::Texture& solidPhi,
                             Renderer::Texture& liquidPhi)
{
    // TODO clear z and other?

    matrixMultiplyBound = matrixMultiply.Bind({A, s, z});
    multiplyAddPBound = multiplyAdd.Bind({pressure, s, alpha, pressure});

    mNormalSolveInit.Record([&](vk::CommandBuffer commandBuffer)
    {
        // r = b
        r.CopyFrom(commandBuffer, b);

        // calculate error
        reduceMaxBound.Record(commandBuffer);

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

        // alpha = rho / zTs
        multiplyZBound.Record(commandBuffer);
        inner.Barrier(commandBuffer, vk::AccessFlagBits::eShaderWrite, vk::AccessFlagBits::eShaderRead);
        reduceSumSigmaBound.Record(commandBuffer);
        sigma.Barrier(commandBuffer, vk::AccessFlagBits::eShaderWrite, vk::AccessFlagBits::eShaderRead);
        divideRhoBound.Record(commandBuffer);
        alpha.Barrier(commandBuffer, vk::AccessFlagBits::eShaderWrite, vk::AccessFlagBits::eShaderRead);

        // p = p + alpha * s
        matrixMultiplyBound.Record(commandBuffer);
        pressure.Barrier(commandBuffer, vk::AccessFlagBits::eShaderWrite, vk::AccessFlagBits::eShaderRead);

        // r = r - alpha * z
        multiplySubRBound.Record(commandBuffer);
        r.Barrier(commandBuffer, vk::AccessFlagBits::eShaderWrite, vk::AccessFlagBits::eShaderRead);

        // calculate max error
        reduceMaxBound.Record(commandBuffer);

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
        rho_new.CopyFrom(commandBuffer, rho);
    });
}

void ConjugateGradient::Solve(Parameters& params)
{
    /*
    // r = b
    r = residual(data.Pressure);

    // if r = 0, return
    error = reduceMax(r);
    // FIXME can use this value to improve tolerance checking as in pcg_solver.h
    if (errorReader.Read().GetFloat(0, 0) == 0.0f)
    {
        return;
    }

    // p = 0
    //data.Pressure.Clear(glm::vec4(0.0f));

    // z = M^-1 r
    ApplyPreconditioner(data);

    // s = z
    s = identity(z.Pressure);

    // rho = zTr
    InnerProduct(rho, z.Pressure, r);

    for (unsigned i = 0 ;; ++i)
    {
        // z = As
        z.Pressure = matrixMultiply(s, data.Weights, data.Diagonal);

        // alpha = rho / zTs
        InnerProduct(sigma, z.Pressure, s);
        alpha = scalarDivision(rho, sigma);

        // p = p + alpha * s
        data.Pressure.Swap();
        data.Pressure = multiplyAdd(Back(data.Pressure), s, alpha);

        // r = r - alpha * z
        r.Swap();
        r = multiplySub(Back(r), z.Pressure, alpha);

        // FIXME don't calculate if we don't do by error
        // calculate max error
        error = reduceMax(r);

        // exit condition
        params.OutIterations = i;
        params.OutError = errorReader.GetFloat(0, 0);
        if (params.IsFinished(i, params.OutError))
        {
            return;
        }

        // async copy of error to client
        errorReader.Read();

        // z = M^-1 r
        ApplyPreconditioner(data);

        // rho_new = zTr
        InnerProduct(rho_new, z.Pressure, r);

        // beta = rho_new / rho
        beta = scalarDivision(rho_new, rho);

        // s = z + beta * s
        s.Swap();
        s = multiplyAdd(z.Pressure, Back(s), beta);

        // rho = rho_new
        rho = identity(rho_new);
    }
    */
}

void ConjugateGradient::NormalSolve(Parameters& params)
{
    mNormalSolveInit.Submit();
    mErrorRead.Submit();

    for (unsigned i = 0 ;; ++i)
    {
        mErrorRead.Wait();

        // exit condition
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

void ConjugateGradient::ApplyPreconditioner(Data& data)
{
}

void ConjugateGradient::InnerProduct(Renderer::Buffer& output, Renderer::Buffer& input1, Renderer::Buffer& input2)
{
    //output = reduceSum(input1, input2);
}

}}
