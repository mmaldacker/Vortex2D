//
//  ConjugateGradient.cpp
//  Vertex2D
//

#include "ConjugateGradient.h"

#include <Vortex2D/Renderer/Disable.h>

namespace Vortex2D { namespace Fluid {

namespace
{

const char * DivideFrag = GLSL(
    in vec2 v_texCoord;
    out vec4 colour_out;

    uniform sampler2D u_x;
    uniform sampler2D u_y;

    void main()
    {
        float x = texture(u_x, v_texCoord).x;
        float y = texture(u_y, v_texCoord).x;

        colour_out = vec4(x / y, 0.0, 0.0, 0.0);
    }
);

const char * MultiplyAddFrag = GLSL(
    in vec2 v_texCoord;
    out vec4 colour_out;

    uniform sampler2D u_x;
    uniform sampler2D u_y;
    uniform sampler2D u_scalar;


    void main()
    {
        float x = texture(u_x, v_texCoord).x;
        float y = texture(u_y, v_texCoord).x;
        float alpha = texture(u_scalar, vec2(0.5)).x;

        colour_out = vec4(x + alpha * y, 0.0, 0.0, 0.0);
    }
);

const char * MultiplyMatrixFrag = GLSL(
    in vec2 v_texCoord;
    out vec4 colour_out;

    uniform sampler2D u_texture;
    uniform sampler2D u_weights;
    uniform sampler2D u_diagonals;

    void main()
    {
        float x = texture(u_texture, v_texCoord).x;
        float d = texture(u_diagonals, v_texCoord).x;

        vec4 p;
        p.x = textureOffset(u_texture, v_texCoord, ivec2(1,0)).x;
        p.y = textureOffset(u_texture, v_texCoord, ivec2(-1,0)).x;
        p.z = textureOffset(u_texture, v_texCoord, ivec2(0,1)).x;
        p.w = textureOffset(u_texture, v_texCoord, ivec2(0,-1)).x;

        vec4 weights = texture(u_weights, v_texCoord);

        float multiply = d * x + dot(p, weights);

        colour_out = vec4(multiply, 0.0, 0.0, 0.0);
    }
);

const char * MultiplySubFrag = GLSL(
    in vec2 v_texCoord;
    out vec4 colour_out;

    uniform sampler2D u_x;
    uniform sampler2D u_y;
    uniform sampler2D u_scalar;

    void main()
    {
        float x = texture(u_x, v_texCoord).x;
        float y = texture(u_y, v_texCoord).x;
        float alpha = texture(u_scalar, vec2(0.5)).x;

        colour_out = vec4(x - alpha * y, 0.0, 0.0, 0.0);
    }
);

const char * ResidualFrag = GLSL(
    in vec2 v_texCoord;
    out vec4 colour_out;

    uniform sampler2D u_texture;

    void main()
    {
        float div = texture(u_texture, v_texCoord).y;
        colour_out = vec4(div, 0.0, 0.0, 0.0);
    }
);

}

using Renderer::Back;

ConjugateGradient::ConjugateGradient(const glm::vec2& size)
    : r(size, 1, true)
    , s(size, 1, true)
    , z(size, 1, false, true)
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
{
    residual.Use().Set("u_texture", 0).Unuse();
    identity.Use().Set("u_texture", 0).Unuse();
    matrixMultiply.Use().Set("u_texture", 0).Set("u_weights", 1).Set("u_diagonals", 2).Unuse();
    scalarDivision.Use().Set("u_x", 0).Set("u_y", 1).Unuse();
    multiplyAdd.Use().Set("u_x", 0).Set("u_y", 1).Set("u_scalar", 2).Unuse();
    multiplySub.Use().Set("u_x", 0).Set("u_y", 1).Set("u_scalar", 2).Unuse();
}

ConjugateGradient::~ConjugateGradient()
{
}

void ConjugateGradient::Build(Renderer::Operator& diagonals,
                              Renderer::Operator& weights,
                              Renderer::Buffer& solidPhi,
                              Renderer::Buffer& liquidPhi)
{
}

void ConjugateGradient::Init(Data& data)
{
    z.Clear(glm::vec4(0.0f));
    RenderMask(z, data);
}

void ConjugateGradient::Solve(Data& data, Parameters& params)
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
    data.Pressure.Clear(glm::vec4(0.0f));

    // z = M^-1 r
    ApplyPreconditioner(data);

    // s = z
    s = identity(z);

    // rho = zTr
    InnerProduct(rho, z, r);

    for (unsigned i = 0 ;; ++i)
    {
        // z = As
        z = matrixMultiply(s, data.Weights, data.Diagonal);

        // alpha = rho / zTs
        InnerProduct(sigma, z, s);
        alpha = scalarDivision(rho, sigma);

        // p = p + alpha * s
        data.Pressure.Swap();
        data.Pressure = multiplyAdd(Back(data.Pressure), s, alpha);

        // r = r - alpha * z
        r.Swap();
        r = multiplySub(Back(r), z, alpha);

        // calculate max error
        error = reduceMax(r);

        // exit condition
        params.OutIterations = i;
        if (params.IsFinished(i, errorReader.GetFloat(0, 0)))
        {
            return;
        }

        // async copy of error to client
        errorReader.Read();

        // z = M^-1 r
        ApplyPreconditioner(data);

        // rho_new = zTr
        InnerProduct(rho_new, z, r);

        // beta = rho_new / rho
        beta = scalarDivision(rho_new, rho);

        // s = z + beta * s
        s.Swap();
        s = multiplyAdd(z, Back(s), beta);

        // rho = rho_new
        rho = identity(rho_new);
    }
}

void ConjugateGradient::NormalSolve(Data& data, Parameters& params)
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
    data.Pressure.Clear(glm::vec4(0.0f));

    // s = r
    s = identity(r);

    // rho = rTr
    InnerProduct(rho, r, r);

    for (unsigned i = 0 ;; ++i)
    {
        // z = As
        z = matrixMultiply(s, data.Weights, data.Diagonal);

        // alpha = rho / zTs
        InnerProduct(sigma, z, s);
        alpha = scalarDivision(rho, sigma);

        // p = p + alpha * s
        data.Pressure.Swap();
        data.Pressure = multiplyAdd(Back(data.Pressure), s, alpha);

        // r = r - alpha * z
        r.Swap();
        r = multiplySub(Back(r), z, alpha);

        // calculate max error
        error = reduceMax(r);
        errorReader.Read();

        // exit condition
        params.OutIterations = i;
        if (params.IsFinished(i, errorReader.GetFloat(0, 0)))
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
    Renderer::Enable e(GL_STENCIL_TEST);
    glStencilMask(0x00);
    glStencilFunc(GL_NOTEQUAL, 1, 0xFF);

    z = scalarDivision(r, data.Diagonal);
}

void ConjugateGradient::InnerProduct(Renderer::Buffer& output, Renderer::Buffer& input1, Renderer::Buffer& input2)
{
    output = reduceSum(input1, input2);
}

}}
