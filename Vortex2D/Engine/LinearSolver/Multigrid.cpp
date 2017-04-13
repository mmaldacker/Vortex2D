//
//  Multigrid.cpp
//  Vortex2D
//

#include "Multigrid.h"

#include <Vortex2D/Renderer/Disable.h>

namespace Vortex2D { namespace Fluid {

namespace
{

const char * DampedJacobiFrag = GLSL(
    in vec2 v_texCoord;
    out vec4 out_color;

    uniform sampler2D u_texture; // this is the pressure
    uniform sampler2D u_weights;
    uniform sampler2D u_diagonals;

    const float w = 2.0/3.0;

    void main()
    {
        // cell.x is pressure and cell.y is div
        vec2 cell = texture(u_texture, v_texCoord).xy;

        vec4 p;
        p.x = textureOffset(u_texture, v_texCoord, ivec2(1,0)).x;
        p.y = textureOffset(u_texture, v_texCoord, ivec2(-1,0)).x;
        p.z = textureOffset(u_texture, v_texCoord, ivec2(0,1)).x;
        p.w = textureOffset(u_texture, v_texCoord, ivec2(0,-1)).x;

        vec4 c = texture(u_weights, v_texCoord);
        float d = texture(u_diagonals, v_texCoord).x;

        float pressure = mix(cell.x, (cell.y - dot(p, c)) / d, w);

        out_color = vec4(pressure, cell.y, 0.0, 0.0);
    }
);

const char * ResidualFrag = GLSL(
    in vec2 v_texCoord;
    out vec4 colour_out;

    uniform sampler2D u_texture;
    uniform sampler2D u_weights;
    uniform sampler2D u_diagonals;

    void main()
    {
        // cell.x is pressure and cell.y is div
        vec2 cell = texture(u_texture, v_texCoord).xy;

        vec4 p;
        p.x = textureOffset(u_texture, v_texCoord, ivec2(1,0)).x;
        p.y = textureOffset(u_texture, v_texCoord, ivec2(-1,0)).x;
        p.z = textureOffset(u_texture, v_texCoord, ivec2(0,1)).x;
        p.w = textureOffset(u_texture, v_texCoord, ivec2(0,-1)).x;

        vec4 c = texture(u_weights, v_texCoord);
        float d = texture(u_diagonals, v_texCoord).x;

        float residual = cell.y - (dot(p, c) + d * cell.x);
        colour_out = vec4(residual, cell.y, 0.0, 0.0);
    }
);

const char* ScaleVert = GLSL(
    in vec2 a_Position;
    in vec2 a_TexCoords;
    out vec2 v_texCoord;

    uniform mat4 u_Projection;
    uniform sampler2D u_texture;

    const vec2 off = vec2(0.5);

    void main()
    {
        gl_Position = u_Projection * vec4(a_Position, 0.0, 1.0);

        vec2 h = textureSize(u_texture, 0);
        vec2 k = ceil(h/vec2(2.0));

        v_texCoord = (vec2(2.0) * (a_TexCoords * k - off) + off) / h;
    }
);

const char* ScaleFrag = GLSL(
    in vec2 v_texCoord;
    out vec4 colour_out;

    uniform sampler2D u_texture;

    void main()
    {
        vec4 value;
        value.x = texture(u_texture, v_texCoord).x;
        value.y = textureOffset(u_texture, v_texCoord, ivec2(1,0)).x;
        value.z = textureOffset(u_texture, v_texCoord, ivec2(0,1)).x;
        value.w = textureOffset(u_texture, v_texCoord, ivec2(1,1)).x;

        colour_out = vec4(0.125 * dot(value, vec4(1.0)), 0.0, 0.0, 0.0);
    }
);

const char* BoundaryMaskFrag = GLSL(

    uniform sampler2D u_texture;
    uniform float u_eveness;

    void main()
    {
        ivec2 pos = ivec2(gl_FragCoord.xy - 0.5);

        float value = 1.0f;
        for (int i = -1; i <= 1; i++)
        {
            for (int j = -1; j <= 1; j++)
            {
                value *= texelFetch(u_texture, pos + ivec2(i,j), 0).x;
            }
        }

        if (value == 0.0)
        {
            if (mod(gl_FragCoord.x + gl_FragCoord.y, 2.0) == u_eveness)
            {
                discard;
            }
        }
        else
        {
            discard;
        }
    }

);

}

using Renderer::Back;

Multigrid::Multigrid(glm::vec2 size)
    : mDepths(0)
    , mResidual(Renderer::Shader::TexturePositionVert, ResidualFrag)
    , mDampedJacobi(Renderer::Shader::TexturePositionVert, DampedJacobiFrag)
    , mIdentity(Renderer::Shader::TexturePositionVert, Renderer::Shader::TexturePositionFrag)
    , mScale(ScaleVert, ScaleFrag)
    , mBoundaryMask(Renderer::Shader::PositionVert, BoundaryMaskFrag)
{
    const float min_size = 4.0f;

    while (size.x > min_size && size.y > min_size)
    {
        size = glm::ceil(size/glm::vec2(2.0f));

        mDatas.emplace_back(size);

        mLiquidPhis.emplace_back(size, 1);
        mSolidPhis.emplace_back(glm::vec2(2)*size, 1);

        mSolidPhis.back().BorderColour(glm::vec4(-1.0f));
        mLiquidPhis.back().BorderColour(glm::vec4(-1.0f));

        mDepths++;
    }

    mResidual.Use().Set("u_texture", 0).Set("u_weights", 1).Set("u_diagonals", 2);
    mDampedJacobi.Use().Set("u_texture", 0).Set("u_weights", 1).Set("u_diagonals", 2);
    mIdentity.Use().Set("u_texture", 0);
    mScale.Use().Set("u_texture", 0);
    mBoundaryMask.Use().Set("u_texture", 0);

    mGaussSeidel.SetW(1.0f);
}

void Multigrid::Smoother(Data& data, int iterations)
{
    for (int i = 0; i < iterations; i++)
    {
        data.Pressure.Swap();
        data.Pressure = mDampedJacobi(Back(data.Pressure), data.Weights, data.Diagonal);
    }
}

void Multigrid::BorderSmoother(Data& data, int iterations, bool up)
{
    for (int i = 0; i < iterations; i++)
    {
        if (up)
        {
            mGaussSeidel.Step(data, 0x02, 0x05);
            mGaussSeidel.Step(data, 0x04, 0x03);
        }
        else
        {
            mGaussSeidel.Step(data, 0x04, 0x03);
            mGaussSeidel.Step(data, 0x02, 0x05);
        }
    }
}

void Multigrid::RenderBoundaryMask(Data& data, Renderer::Buffer& buffer)
{
    Renderer::Enable e(GL_STENCIL_TEST);
    Renderer::DisableColorMask c;

    glStencilFunc(GL_NOTEQUAL, 0, 0xFF); // write value in stencil buffer
    glStencilOp(GL_KEEP, GL_KEEP, GL_INVERT); // invert value
    glStencilMask(0x03); // write 2 in stencil

    mBoundaryMask.Use().Set("u_eveness", 1.0f);

    data.Pressure = mBoundaryMask(buffer);
    data.Pressure.Swap();
    data.Pressure = mBoundaryMask(buffer);
    data.Pressure.Swap();

    glStencilMask(0x05); // write 4 in stencil

    mBoundaryMask.Use().Set("u_eveness", 0.0f);

    data.Pressure = mBoundaryMask(buffer);
    data.Pressure.Swap();
    data.Pressure = mBoundaryMask(buffer);
    data.Pressure.Swap();

    glStencilMask(0x00); // disable stencil writing
}

void Multigrid::Build(Data& data,
                      Renderer::Operator& diagonals,
                      Renderer::Operator& weights,
                      Renderer::Buffer& solidPhi,
                      Renderer::Buffer& liquidPhi)
{
    data.Diagonal = diagonals(solidPhi, liquidPhi);
    data.Weights = weights(solidPhi, liquidPhi);

    mLiquidPhis[0] = mScale(liquidPhi);
    mSolidPhis[0] = mScale(solidPhi);

    for (int i = 1; i < mDepths; i++)
    {
        mLiquidPhis[i - 1].ClampToEdge();
        mLiquidPhis[i] = mScale(mLiquidPhis[i - 1]);

        mSolidPhis[i - 1].ClampToEdge();
        mSolidPhis[i] = mScale(mSolidPhis[i - 1]);
    }

    for (int i = 0; i < mDepths; i++)
    {
        auto& x = mDatas[i];

        mSolidPhis[i].ClampToBorder();
        mLiquidPhis[i].ClampToBorder();

        x.Diagonal = diagonals(mSolidPhis[i], mLiquidPhis[i]);
        x.Weights = weights(mSolidPhis[i], mLiquidPhis[i]);
    }
}

void Multigrid::Init(LinearSolver::Data& data)
{
    RenderMask(data.Pressure, data);
    RenderBoundaryMask(data, data.Diagonal);

    for (int i = 0; i < mDepths; i++)
    {
        mDatas[i].Pressure.Clear(glm::vec4(0.0f));
        RenderMask(mDatas[i].Pressure, mDatas[i]);
        RenderBoundaryMask(mDatas[i], mDatas[i].Diagonal);
    }
}

void Multigrid::Solve(LinearSolver::Data& data, Parameters&)
{
    Renderer::Enable e(GL_STENCIL_TEST);
    glStencilMask(0x00);
    glStencilFunc(GL_NOTEQUAL, 0, 0xFF);

    int numIterations = 2;

    Smoother(data, numIterations);
    //BorderSmoother(data, numIterations, true);

    if (mDepths > 0)
    {
        data.Pressure.Swap();
        data.Pressure = mResidual(Back(data.Pressure), data.Weights, data.Diagonal);

        mDatas[0].Pressure = mTransfer.Restrict(data.Pressure);

        for (int i = 0 ; i < mDepths - 1 ; i++)
        {
            numIterations *= 2;
            auto& x = mDatas[i];

            Smoother(x, numIterations);
            //BorderSmoother(x, numIterations, true);

            x.Pressure.Swap();
            x.Pressure = mResidual(Back(x.Pressure), x.Weights, x.Diagonal);

            mDatas[i + 1].Pressure = mTransfer.Restrict(x.Pressure);
        }

        Smoother(mDatas.back(), numIterations * 2);

        for (int i = mDepths - 2 ; i >= 0 ; --i)
        {
            auto& x = mDatas[i];

            x.Pressure.Swap();
            x.Pressure = mTransfer.Prolongate(mDatas[i + 1].Pressure, Back(x.Pressure));

            //BorderSmoother(x, numIterations, false);
            Smoother(x, numIterations);

            numIterations /= 2;
        }

        data.Pressure.Swap();
        data.Pressure = mTransfer.Prolongate(mDatas[0].Pressure, Back(data.Pressure));
    }

    //BorderSmoother(data, numIterations, false);
    Smoother(data, numIterations);
}

}}
