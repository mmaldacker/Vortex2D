//
//  ConjugateGradient.cpp
//  Vertex2D
//

#include "ConjugateGradient.h"
#include "Disable.h"

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

ConjugateGradient::ConjugateGradient(const glm::vec2& size, unsigned iterations)
    : r(size, 1, true)
    , s(size, 1, true)
    , z(size, 1, false, true)
    , alpha({1,1}, 1)
    , beta({1,1}, 1)
    , rho({1,1}, 1)
    , rho_new({1,1}, 1)
    , sigma({1,1}, 1)
    , matrixMultiply(Renderer::Shader::TexturePositionVert, MultiplyMatrixFrag)
    , scalarDivision(Renderer::Shader::TexturePositionVert, DivideFrag)
    , multiplyAdd(Renderer::Shader::TexturePositionVert, MultiplyAddFrag)
    , multiplySub(Renderer::Shader::TexturePositionVert, MultiplySubFrag)
    , residual(Renderer::Shader::TexturePositionVert, ResidualFrag)
    , identity(Renderer::Shader::TexturePositionVert, Renderer::Shader::TexturePositionFrag)
    , reduce(size)
    , mIterations(iterations)
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

void ConjugateGradient::Init(LinearSolver::Data& data)
{
    RenderMask(z, data);
}

void ConjugateGradient::Solve(LinearSolver::Data& data)
{
    // r = b
    r = residual(data.Pressure);

    // p = 0
    data.Pressure.Clear(glm::vec4(0.0f));

    // z = M^-1 r
    ApplyPreconditioner(data);

    // s = z
    s = identity(z);

    // rho = zTr
    rho = reduce(z, r);

    for (unsigned i = 0 ; i < mIterations; ++i)
    {
        // z = As
        z = matrixMultiply(s, data.Weights, data.Diagonal);

        // alpha = rho / zTs
        sigma = reduce(z, s);
        alpha = scalarDivision(rho, sigma);

        // p = p + alpha * s
        data.Pressure.Swap();
        data.Pressure = multiplyAdd(Back(data.Pressure), s, alpha);

        // r = r - alpha * z
        r.Swap();
        r = multiplySub(Back(r), z, alpha);

        // z = M^-1 r
        ApplyPreconditioner(data);

        // rho_new = zTr
        rho_new = reduce(z, r);

        // beta = rho_new / rho
        beta = scalarDivision(rho_new, rho);

        // s = z + beta * s
        s.Swap();
        s = multiplyAdd(z, Back(s), beta);

        // rho = rho_new
        rho = identity(rho_new);
    }
}

void ConjugateGradient::NormalSolve(LinearSolver::Data& data)
{
    // r = b
    r = residual(data.Pressure);

    // p = 0
    data.Pressure.Clear(glm::vec4(0.0f));

    // s = r
    s = identity(r);

    // rho = rTr
    rho = reduce(r, r);

    for (unsigned i = 0 ; i < mIterations; ++i)
    {
        // z = As
        z = matrixMultiply(s, data.Weights, data.Diagonal);

        // alpha = rho / zTs
        sigma = reduce(z, s);
        alpha = scalarDivision(rho, sigma);

        // p = p + alpha * s
        data.Pressure.Swap();
        data.Pressure = multiplyAdd(Back(data.Pressure), s, alpha);

        // r = r - alpha * z
        r.Swap();
        r = multiplySub(Back(r), z, alpha);

        // rho_new = rTr
        rho_new = reduce(r, r);

        // beta = rho_new / rho
        beta = scalarDivision(rho_new, rho);

        // s = r + beta * s
        s.Swap();
        s = multiplyAdd(r, Back(s), beta);

        // rho = rho_new
        rho = identity(rho_new);
    }
}

void ConjugateGradient::ApplyPreconditioner(LinearSolver::Data& data)
{
    Renderer::Enable e(GL_STENCIL_TEST);
    glStencilMask(0x00);
    glStencilFunc(GL_NOTEQUAL, 1, 0xFF);

    z = scalarDivision(r, data.Diagonal);
}

}}
