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

const char * ProlongateFrag = GLSL(
    in vec2 v_texCoord;

    uniform sampler2D u_texture;
    uniform sampler2D u_pressure;

    out vec4 colour_out;

    void main()
    {
        float x = texture(u_texture, v_texCoord).x;
        vec2 p = texture(u_pressure, v_texCoord).xy;

        colour_out = vec4(p.x + x, p.y, 0.0, 0.0);
    }
);

const char * RestrictFrag = GLSL(
    in vec2 v_texCoord;

    uniform sampler2D u_texture;

    out vec4 colour_out;

    void main()
    {
        float p = texture(u_texture, v_texCoord).x;

        colour_out = vec4(0.0, p, 0.0, 0.0);
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

const char * MaxFrag = GLSL(
    out vec4 colour_out;

    uniform sampler2D u_texture;

    void main()
    {
        ivec2 pos = 2 * ivec2(gl_FragCoord.xy - 0.5);

        vec4 value;
        value.x = texelFetch(u_texture, pos + ivec2(0,0), 0).x;
        value.y = texelFetch(u_texture, pos + ivec2(1,0), 0).x;
        value.z = texelFetch(u_texture, pos + ivec2(0,1), 0).x;
        value.w = texelFetch(u_texture, pos + ivec2(1,1), 0).x;

        colour_out = vec4(max(max(value.x, value.y), max(value.z, value.w)), 0.0, 0.0, 0.0);
    }
);

const char * MinFrag = GLSL(
    out vec4 colour_out;

    uniform sampler2D u_texture;

    void main()
    {
        ivec2 pos = 2 * ivec2(gl_FragCoord.xy - 0.5);

        vec4 value;
        value.x = texelFetch(u_texture, pos + ivec2(0,0), 0).x;
        value.y = texelFetch(u_texture, pos + ivec2(1,0), 0).x;
        value.z = texelFetch(u_texture, pos + ivec2(0,1), 0).x;
        value.w = texelFetch(u_texture, pos + ivec2(1,1), 0).x;

        colour_out = vec4(min(min(value.x, value.y), min(value.z, value.w)), 0.0, 0.0, 0.0);
    }
);

}

using Renderer::Back;

Multigrid::Multigrid(glm::vec2 size)
    : mDepths(0)
    , mProlongate(Renderer::Shader::TexturePositionVert, ProlongateFrag)
    , mResidual(Renderer::Shader::TexturePositionVert, ResidualFrag)
    , mRestrict(Renderer::Shader::TexturePositionVert, RestrictFrag)
    , mDampedJacobi(Renderer::Shader::TexturePositionVert, DampedJacobiFrag)
    , mIdentity(Renderer::Shader::TexturePositionVert, Renderer::Shader::TexturePositionFrag)
    , mMax(Renderer::Shader::TexturePositionVert, MaxFrag)
    , mMin(Renderer::Shader::TexturePositionVert, MinFrag)
{
    const float min_size = 4.0f;

    while (size.x > min_size && size.y > min_size)
    {
        size = glm::ceil(size/glm::vec2(2.0f));

        mDatas.emplace_back(size);
        mDatas.back().Pressure.Linear();

        mLiquidPhis.emplace_back(size, 1);
        mSolidPhis.emplace_back(glm::vec2(2)*size, 1);

        mDepths++;
    }

    mProlongate.Use().Set("u_texture", 0).Set("u_pressure", 1).Unuse();
    mResidual.Use().Set("u_texture", 0).Set("u_weights", 1).Set("u_diagonals", 2).Unuse();
    mRestrict.Use().Set("u_texture", 0).Unuse();
    mDampedJacobi.Use().Set("u_texture", 0).Set("u_weights", 1).Set("u_diagonals", 2).Unuse();
    mIdentity.Use().Set("u_texture", 0).Unuse();
    mMax.Use().Set("u_texture", 0).Unuse();
    mMin.Use().Set("u_texture", 0).Unuse();
}

void Multigrid::DampedJacobi(Data& data, int iterations)
{
    for(int i = 0 ; i < iterations ; i++)
    {
        data.Pressure.Swap();
        data.Pressure = mDampedJacobi(Back(data.Pressure), data.Weights, data.Diagonal);
    }
}

void Multigrid::Build(Renderer::Operator& diagonals,
                      Renderer::Operator& weights,
                      Renderer::Buffer& solidPhi,
                      Renderer::Buffer& liquidPhi)
{
    mLiquidPhis[0] = mMax(liquidPhi);
    mSolidPhis[0] = mMin(solidPhi);

    for (int i = 1; i < mDepths; i++)
    {
        mLiquidPhis[i] = mMax(mLiquidPhis[i - 1]);
        mSolidPhis[i] = mMin(mSolidPhis[i - 1]);
    }

    for (int i = 0; i < mDepths; i++)
    {
        auto& x = mDatas[i];
        x.Diagonal = diagonals(mSolidPhis[i], mLiquidPhis[i]);
        x.Weights = weights(mSolidPhis[i], mLiquidPhis[i]);
    }
}

void Multigrid::Init(LinearSolver::Data& data)
{
    RenderMask(data.Pressure, data);

    for (int i = 0; i < mDepths; i++)
    {
        RenderMask(mDatas[i].Pressure, mDatas[i]);
    }
}

void Multigrid::Solve(LinearSolver::Data& data, Parameters&)
{
    assert(!mDatas.empty());

    Renderer::Enable e(GL_STENCIL_TEST);
    glStencilMask(0x00);
    glStencilFunc(GL_NOTEQUAL, 1, 0xFF);

    int numIterations = 2;

    DampedJacobi(data, numIterations);

    data.Pressure.Swap();
    data.Pressure = mResidual(Back(data.Pressure), data.Weights, data.Diagonal);

    mDatas[0].Pressure = mRestrict(data.Pressure);

    for (int i = 0 ; i < mDepths - 1 ; i++)
    {
        numIterations *= 2;
        auto& x = mDatas[i];

        DampedJacobi(x, numIterations);

        x.Pressure.Swap();
        x.Pressure = mResidual(Back(x.Pressure), x.Weights, x.Diagonal);

        mDatas[i + 1].Pressure = mRestrict(x.Pressure);
    }

    DampedJacobi(mDatas.back(), numIterations * 2);

    for(int i = mDepths - 2 ; i >= 0 ; --i)
    {
        auto& x = mDatas[i];

        x.Pressure.Swap();
        x.Pressure = mProlongate(mDatas[i + 1].Pressure, Back(x.Pressure));

        DampedJacobi(x, numIterations);

        numIterations /= 2;
    }

    auto& u = mDatas[0];

    data.Pressure.Swap();
    data.Pressure = mProlongate(u.Pressure, Back(data.Pressure));

    DampedJacobi(data, numIterations);
}

}}
