//
//  Multigrid.cpp
//  Vortex2D
//

#include "Multigrid.h"

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

    float pressure = 0.0;
    if(d > 0.0) pressure = cell.x + w * (dot(p,c) + cell.y) / d;

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

    float residual = dot(p,c) - d * cell.x + cell.y;
    colour_out = vec4(residual, cell.y, 0.0, 0.0);
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
{
    const float min_size = 4.0f;

    do
    {
        mDatas.emplace_back(size);
        mDatas.back().Pressure.Linear();
        mDatas.back().Pressure.Clear(glm::vec4(0.0));
        mDatas.back().Pressure.ClearStencil();
        size = glm::ceil(size/glm::vec2(2.0f));
        mDepths++;
    } while (size.x > min_size && size.y > min_size);

    mProlongate.Use().Set("u_texture", 0).Set("u_pressure", 1).Unuse();
    mResidual.Use().Set("u_texture", 0).Set("u_weights", 1).Set("u_diagonals", 2).Unuse();
    mRestrict.Use().Set("u_texture", 0).Unuse();
    mDampedJacobi.Use().Set("u_texture", 0).Set("u_weights", 1).Set("u_diagonals", 2).Unuse();
    mIdentity.Use().Set("u_texture", 0).Unuse();
}

LinearSolver::Data & Multigrid::GetData(int depth)
{
    return mDatas[depth];
}

void Multigrid::DampedJacobi(Data & data, int iterations)
{
    for(int i = 0 ; i < iterations ; i++)
    {
        data.Pressure.Swap();
        data.Pressure = mDampedJacobi(Back(data.Pressure), data.Weights, data.Diagonal);
    }
}

void Multigrid::Build(Data& data,
                      Renderer::Operator& diagonals,
                      Renderer::Operator& weights,
                      Renderer::Buffer& solidPhi,
                      Renderer::Buffer& liquidPhi)
{

}

void Multigrid::Init(LinearSolver::Data& data)
{
}

void Multigrid::Solve(LinearSolver::Data& data, Parameters& params)
{
    for(int i = 0 ; i < mDepths - 1 ; i++)
    {
        auto & x = GetData(i);

        DampedJacobi(x);

        x.Pressure.Swap();
        x.Pressure = mResidual(Back(x.Pressure), x.Weights, x.Diagonal);

        auto & r = GetData(i+1);
        r.Pressure = mRestrict(x.Pressure);
    }

    DampedJacobi(GetData(mDepths - 1));

    for(int i = mDepths - 2 ; i >= 0 ; --i)
    {
        auto & x = GetData(i);
        auto & u = GetData(i+1);

        x.Pressure.Swap();
        x.Pressure = mProlongate(u.Pressure, Back(x.Pressure));

        DampedJacobi(x);
    }

    data.Pressure = mIdentity(GetData(0).Pressure);
}

}}
