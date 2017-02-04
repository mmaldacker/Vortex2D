//
//  Pressure.cpp
//  Vortex2D
//

#include "Pressure.h"

namespace Vortex2D { namespace Fluid {

namespace
{

const char * DivFrag = GLSL(
    uniform sampler2D u_velocity;
    uniform sampler2D u_obstacles;
    uniform sampler2D u_obstacles_velocity;

    in vec2 v_texCoord;
    out vec4 out_color;

    float fraction_inside(float a, float b)
    {
        if(a < 0.0 && b < 0.0)
            return 1.0;
        if(a < 0.0 && b >= 0.0)
            return a / (a - b);
        if(a >= 0.0 && b < 0.0)
            return b / (b - a);
        return 0.0;
    }

    void main()
    {
        // FIXME replace with vec4 and use dot product
        vec2  uv  = texture(u_velocity, v_texCoord).xy;
        float uxp = textureOffset(u_velocity, v_texCoord, ivec2(1,0)).x;
        float vyp = textureOffset(u_velocity, v_texCoord, ivec2(0,1)).y;

        float c   = texture(u_obstacles, v_texCoord).x;
        float cxp = textureOffset(u_obstacles, v_texCoord, ivec2(2,0)).x;
        float cxn = textureOffset(u_obstacles, v_texCoord, ivec2(-2,0)).x;
        float cyp = textureOffset(u_obstacles, v_texCoord, ivec2(0,2)).x;
        float cyn = textureOffset(u_obstacles, v_texCoord, ivec2(0,-2)).x;

        float h_cxn = fraction_inside(c, cxn);
        float h_cyn = fraction_inside(c, cyn);
        float h_cxp = fraction_inside(c, cxp);
        float h_cyp = fraction_inside(c, cyp);

        float dx = 1.0;
        float div = -(h_cxp * uxp - h_cxn * uv.x + h_cyp * vyp - h_cyn * uv.y) / dx;

        // FIXME what should the value of c be?

        /*
        vec2 solid_uv = texture(u_obstacles_velocity, v_texCoord).xy;
        float solid_uxp = textureOffset(u_obstacles_velocity, v_texCoord, ivec2(1,0)).x;
        float solid_vyp = textureOffset(u_obstacles_velocity, v_texCoord, ivec2(0,1)).y;

        div += ((h_cxp+c-1.0)*solid_uxp - (h_cxn+c-1.0)*solid_uv.x + (h_cyp+c-1.0)*solid_vyp - (h_cyn+c-1.0)*solid_uv.y) / dx;
        */

        //pressure, div, 0, 0
        out_color = vec4(0.0, div, 0.0, 0.0);
    }
);

const char * ProjectFrag = GLSL(
    in vec2 v_texCoord;
    out vec4 out_color;

    uniform sampler2D u_velocity;
    uniform sampler2D u_pressure;
    uniform sampler2D u_fluid;
    uniform sampler2D u_obstacles;
    uniform sampler2D u_obstacles_velocity;
    uniform float delta;

    float fraction_inside(float a, float b)
    {
        if(a < 0.0 && b < 0.0)
            return 1.0;
        if(a < 0.0 && b >= 0.0)
            return a / (a - b);
        if(a >= 0.0 && b < 0.0)
            return b / (b - a);
        return 0.0;
    }

    void main()
    {
        vec2 cell = texture(u_velocity, v_texCoord).xy;

        float p = texture(u_pressure, v_texCoord).x;
        float pxn = textureOffset(u_pressure, v_texCoord, ivec2(-1,0)).x;
        float pyn = textureOffset(u_pressure, v_texCoord, ivec2(0,-1)).x;

        vec2 pGrad = vec2(p-pxn, p-pyn);

        float phi = texture(u_fluid, v_texCoord).x;
        float phixn = textureOffset(u_fluid, v_texCoord, ivec2(-1,0)).x;
        float phiyn = textureOffset(u_fluid, v_texCoord, ivec2(0,-1)).x;

        vec2 theta = vec2(fraction_inside(phi, phixn), fraction_inside(phi, phiyn));
        pGrad /= max(theta, 0.01);

        vec2 mask = vec2(1.0);
        vec2 obsV = vec2(0.0);

        if (textureOffset(u_obstacles, v_texCoord, ivec2(2,0)).x > 0.0)
        {
            mask.x = 0.0;
            obsV.x = textureOffset(u_obstacles_velocity, v_texCoord, ivec2(1,0)).x;
        }
        if (textureOffset(u_obstacles, v_texCoord, ivec2(-2,0)).x > 0.0)
        {
            mask.x = 0.0;
            obsV.x = textureOffset(u_obstacles_velocity, v_texCoord, ivec2(-1,0)).x;
        }
        if (textureOffset(u_obstacles, v_texCoord, ivec2(0,2)).x > 0.0)
        {
            mask.y = 0.0;
            obsV.y = textureOffset(u_obstacles_velocity, v_texCoord, ivec2(0,1)).y;
        }
        if (textureOffset(u_obstacles, v_texCoord, ivec2(0,-2)).x > 0.0)
        {
            mask.y = 0.0;
            obsV.y = textureOffset(u_obstacles_velocity, v_texCoord, ivec2(0,-1)).y;
        }

        float dx = 1.0;
        vec2 new_cell = cell - delta * pGrad / dx;
        out_color = vec4(mask * new_cell + obsV, 0.0, 0.0);
    }
);

const char * WeightsFrag = GLSL(
    in vec2 v_texCoord;
    out vec4 out_color;

    uniform sampler2D u_obstacles;
    uniform sampler2D u_fluid;
    uniform float delta;

    float fraction_inside(float a, float b)
    {
        if(a < 0.0 && b < 0.0)
            return 1.0;
        if(a < 0.0 && b >= 0.0)
            return a / (a - b);
        if(a >= 0.0 && b < 0.0)
            return b / (b - a);
        return 0.0;
    }

    void main()
    {
        float c   = texture(u_obstacles, v_texCoord).x;
        float cxp = textureOffset(u_obstacles, v_texCoord, ivec2(2,0)).x;
        float cxn = textureOffset(u_obstacles, v_texCoord, ivec2(-2,0)).x;
        float cyp = textureOffset(u_obstacles, v_texCoord, ivec2(0,2)).x;
        float cyn = textureOffset(u_obstacles, v_texCoord, ivec2(0,-2)).x;

        float pxp = textureOffset(u_fluid, v_texCoord, ivec2(1,0)).x;
        float pxn = textureOffset(u_fluid, v_texCoord, ivec2(-1,0)).x;
        float pyp = textureOffset(u_fluid, v_texCoord, ivec2(0,1)).x;
        float pyn = textureOffset(u_fluid, v_texCoord, ivec2(0,-1)).x;

        vec4 weights;
        weights.x = pxp >= 0.0 ? 0.0 : fraction_inside(c, cxp);
        weights.y = pxn >= 0.0 ? 0.0 : fraction_inside(c, cxn);
        weights.z = pyp >= 0.0 ? 0.0 : fraction_inside(c, cyp);
        weights.w = pyn >= 0.0 ? 0.0 : fraction_inside(c, cyn);

        float dx = 1.0;
        out_color = delta * weights / (dx*dx);
    }
);

const char * DiagonalsFrag = GLSL(
    in vec2 v_texCoord;
    out vec4 out_color;

    uniform sampler2D u_obstacles;
    uniform sampler2D u_fluid;
    uniform float delta;

    float fraction_inside(float a, float b)
    {
        if(a < 0.0 && b < 0.0)
            return 1.0;
        if(a < 0.0 && b >= 0.0)
            return a / (a - b);
        if(a >= 0.0 && b < 0.0)
            return b / (b - a);
        return 0.0;
    }

    void main()
    {
        float c   = texture(u_obstacles, v_texCoord).x;
        float cxp = textureOffset(u_obstacles, v_texCoord, ivec2(2,0)).x;
        float cxn = textureOffset(u_obstacles, v_texCoord, ivec2(-2,0)).x;
        float cyp = textureOffset(u_obstacles, v_texCoord, ivec2(0,2)).x;
        float cyn = textureOffset(u_obstacles, v_texCoord, ivec2(0,-2)).x;

        vec4 weights;
        weights.x = fraction_inside(c, cxp);
        weights.y = fraction_inside(c, cxn);
        weights.z = fraction_inside(c, cyp);
        weights.w = fraction_inside(c, cyn);

        float p = texture(u_fluid, v_texCoord).x;
        float pxp = textureOffset(u_fluid, v_texCoord, ivec2(1,0)).x;
        float pxn = textureOffset(u_fluid, v_texCoord, ivec2(-1,0)).x;
        float pyp = textureOffset(u_fluid, v_texCoord, ivec2(0,1)).x;
        float pyn = textureOffset(u_fluid, v_texCoord, ivec2(0,-1)).x;

        vec4 theta;
        theta.x = fraction_inside(p, pxp);
        theta.y = fraction_inside(p, pxn);
        theta.z = fraction_inside(p, pyp);
        theta.w = fraction_inside(p, pyn);

        weights /= max(theta, 0.01);

        float dx = 1.0;
        out_color = vec4(delta * dot(weights,vec4(1.0)) / (dx*dx),0.0, 0.0, 0.0);
    }
);

}

using Renderer::Back;

Pressure::Pressure(float dt,
                   LinearSolver& solver,
                   LinearSolver::Data& data,
                   Renderer::Buffer& velocity,
                   Renderer::Buffer& solidPhi,
                   Renderer::Buffer& liquidPhi,
                   Renderer::Buffer& solidVelocity)
    : mSolver(solver)
    , mData(data)
    , mVelocity(velocity)
    , mSolidPhi(solidPhi)
    , mLiquidPhi(liquidPhi)
    , mSolidVelocity(solidVelocity)
    , mDiv(Renderer::Shader::TexturePositionVert, DivFrag)
    , mProject(Renderer::Shader::TexturePositionVert, ProjectFrag)
    , mWeights(Renderer::Shader::TexturePositionVert, WeightsFrag)
    , mDiagonals(Renderer::Shader::TexturePositionVert, DiagonalsFrag)
{
    mDiv.Use().Set("u_velocity", 0).Set("u_obstacles", 1).Set("u_obstacles_velocity", 2).Unuse();
    mProject.Use().Set("u_velocity", 0).Set("u_pressure", 1).Set("u_fluid", 2).Set("u_obstacles", 3).Set("u_obstacles_velocity", 4).Set("delta", dt).Unuse();
    mWeights.Use().Set("u_obstacles", 0).Set("u_fluid", 1).Set("delta", dt).Unuse();
    mDiagonals.Use().Set("u_obstacles", 0).Set("u_fluid", 1).Set("delta", dt).Unuse();
}

void Pressure::Solve(LinearSolver::Parameters& params)
{

    mData.Pressure = mDiv(mVelocity, mSolidPhi, mSolidVelocity);
    mData.Weights = mWeights(mSolidPhi, mLiquidPhi);
    mData.Diagonal = mDiagonals(mSolidPhi, mLiquidPhi);

    mSolver.Init(mData);
    mSolver.Solve(mData, params);

    mVelocity.Swap();
    mVelocity = mProject(Back(mVelocity),
                         mData.Pressure,
                         mLiquidPhi,
                         mSolidPhi,
                         mSolidVelocity);
}

}}
