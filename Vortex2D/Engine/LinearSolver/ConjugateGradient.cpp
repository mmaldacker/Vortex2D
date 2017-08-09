//
//  ConjugateGradient.cpp
//  Vertex2D
//

#include "ConjugateGradient.h"

namespace Vortex2D { namespace Fluid {

ConjugateGradient::ConjugateGradient(const glm::vec2& size)
    : r(size, 1, true)
    , s(size, 1, true)
    , alpha({1,1}, 1)
    , beta({1,1}, 1)
    , rho({1,1}, 1)
    , rho_new({1,1}, 1)
    , sigma({1,1}, 1)
    , error({1,1}, 1)
    , matrixMultiply(Renderer::Shader::TexturePositionVert, MultiplyMatrixFrag)
    , scalarDivision(Renderer::Shader::TexturePositionVert, DivideFrag)
    , multiplyAdd(Renderer::Shader::TexturePositionVert, MultiplyAddFrag)
    , multiplySub(Renderer::Shader::TexturePositionVert, MultiplySubFrag)
    , residual(Renderer::Shader::TexturePositionVert, ResidualFrag)
    , identity(Renderer::Shader::TexturePositionVert, Renderer::Shader::TexturePositionFrag)
    , reduceSum(size)
    , reduceMax(size)
    , errorReader(error)
    , swizzle(Renderer::Shader::TexturePositionVert, SwizzleFrag)
    , z(size)
    , preconditioner(size)
{
}

ConjugateGradient::~ConjugateGradient()
{
}

void ConjugateGradient::Init(Renderer::Buffer& data, Renderer::Buffer& pressure)
{
    // TODO clear z and other?
}

void ConjugateGradient::Solve(Parameters& params)
{
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
}

void ConjugateGradient::NormalSolve(Parameters& params)
{
    // r = b
    r = residual(data.Pressure);

    // if r = 0, return
    error = reduceMax(r);
    if (errorReader.Read().GetFloat(0, 0) == 0.0f)
    {
        return;
    }

    // p = 0
    //data.Pressure.Clear(glm::vec4(0.0f));

    // s = r
    s = identity(r);

    // rho = rTr
    InnerProduct(rho, r, r);

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

        // calculate max error
        error = reduceMax(r);
        errorReader.Read();

        // exit condition
        params.OutIterations = i;
        params.OutError = errorReader.GetFloat(0, 0);
        if (params.IsFinished(i, params.OutError))
        {
            return;
        }

        // rho_new = rTr
        InnerProduct(rho_new, r, r);

        // beta = rho_new / rho
        beta = scalarDivision(rho_new, rho);

        // s = r + beta * s
        s.Swap();
        s = multiplyAdd(r, Back(s), beta);

        // rho = rho_new
        rho = identity(rho_new);
    }
}

void ConjugateGradient::ApplyPreconditioner(Data& data)
{
}

void ConjugateGradient::InnerProduct(Renderer::Buffer& output, Renderer::Buffer& input1, Renderer::Buffer& input2)
{
    output = reduceSum(input1, input2);
}

}}
