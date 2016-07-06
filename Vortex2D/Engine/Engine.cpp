//
//  Engine.cpp
//  Vortex
//

#include "Engine.h"
#include "Disable.h"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>
#include <iostream>

namespace Vortex2D { namespace Fluid {

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
        float phixn = textureOffset(u_fluid, v_texCoord, ivec2(-1, 0)).x;
        float phiyn = textureOffset(u_fluid, v_texCoord, ivec2(0, -1)).x;

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
        float pxn = textureOffset(u_fluid, v_texCoord, ivec2(-1, 0)).x;
        float pyp = textureOffset(u_fluid, v_texCoord, ivec2(0, 1)).x;
        float pyn = textureOffset(u_fluid, v_texCoord, ivec2(0, -1)).x;

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
        float pxn = textureOffset(u_fluid, v_texCoord, ivec2(-1, 0)).x;
        float pyp = textureOffset(u_fluid, v_texCoord, ivec2(0, 1)).x;
        float pyn = textureOffset(u_fluid, v_texCoord, ivec2(0, -1)).x;

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

const char * AdvectFrag = GLSL(
    uniform sampler2D u_texture;
    uniform sampler2D u_velocity;
    uniform float delta;

    in vec2 v_texCoord;
    out vec4 out_color;


    vec4 cubic(vec4 f1, vec4 f2, vec4 f3, vec4 f4, float xd)
    {
       float xd2 = xd * xd;
       float xd3 = xd2 * xd;

       return f1*(     - 0.5*xd  +     xd2 - 0.5*xd3) +
       f2*( 1.0           - 2.5*xd2 + 1.5*xd3) +
       f3*(       0.5*xd  + 2.0*xd2 - 1.5*xd3) +
       f4*(               - 0.5*xd2 + 0.5*xd3);
    }

    vec4 bicubic(sampler2D u_texture, vec2 xy)
    {
       ivec2 ij = ivec2(floor(xy)) - 1;
       vec2 f = xy - (ij + 1);

       vec4 t[16];
       for(int j = 0 ; j < 4 ; ++j)
       {
           for(int i = 0 ; i < 4 ; ++i)
           {
               t[i + 4*j] = texelFetch(u_texture, ij + ivec2(i,j), 0);
           }
       }
       
       vec4 x = cubic(
                      cubic(t[0], t[4], t[8], t[12], f.y),
                      cubic(t[1], t[5], t[9], t[13], f.y),
                      cubic(t[2], t[6], t[10], t[14], f.y),
                      cubic(t[3], t[7], t[11], t[15], f.y),
                      f.x
                      );
       
       vec4 maxValue = max(max(t[5], t[6]), max(t[9], t[10]));
       vec4 minValue = min(min(t[5], t[6]), min(t[9], t[10]));
       
       return clamp(x, minValue, maxValue);
    }

    const float a = 2.0/9.0;
    const float b = 3.0/9.0;
    const float c = 4.0/9.0;

    void main(void)
    {
       vec2 pos = gl_FragCoord.xy - 0.5;
       
       vec2 k1 = texture(u_velocity, v_texCoord).xy;
       vec2 k2 = bicubic(u_velocity, pos - 0.5*delta*k1).xy;
       vec2 k3 = bicubic(u_velocity, pos - 0.75*delta*k2).xy;
       
       out_color = bicubic(u_texture, pos - a*delta*k1 - b*delta*k2 - c*delta*k3);
    }
    );


const char * ExtrapolateFluidFrag = GLSL(
     uniform sampler2D u_fluid;
     uniform sampler2D u_obstacles;

     in vec2 v_texCoord;
     out vec4 out_color;

     void main(void)
     {
         float f = texture(u_fluid, v_texCoord).x;
         float o = texture(u_obstacles, v_texCoord).x;
         if(f <= 1.0 && f >= 0.0 && o >= 0.0)
         {
            out_color = vec4(-1.0, 0.0, 0.0, 0.0);
         }
         else
         {
             out_color = vec4(f, 0.0, 0.0, 0.0);
         }
     }
);

const char * ConstrainVelocityFrag = GLSL(
    uniform sampler2D u_velocity;
    uniform sampler2D u_obstacles;

    in vec2 v_texCoord;
    out vec4 out_color;

    void main(void)
    {
        vec2 uv = texture(u_velocity, v_texCoord).xy;

        float cxp = textureOffset(u_obstacles, v_texCoord, ivec2(2,0)).x;
        float cyp = textureOffset(u_obstacles, v_texCoord, ivec2(0,2)).x;
        float cxn = textureOffset(u_obstacles, v_texCoord, ivec2(-2,0)).x;
        float cyn = textureOffset(u_obstacles, v_texCoord, ivec2(0,-2)).x;

        vec2 cGrad = normalize(vec2(cxp-cxn,cyp-cyn));
        float perp_component = dot(uv, cGrad);

        // FIXME need to set the obstacle velocity (means we don't have to set it in project?)
        out_color = vec4(uv - perp_component*cGrad, 0.0, 0.0);
    }
);

const char * FluidFrag = GLSL(
    in vec2 v_texCoord;
    uniform sampler2D u_texture;
    uniform vec4 u_Colour;

    out vec4 out_color;

    void main()
    {
        float x = texture(u_texture, v_texCoord).x;
        if(x < 0.0)
        {
            out_color = u_Colour;
        }
        else
        {
            out_color = vec4(0.0);
        }
    }
);

Engine::Engine(Dimensions dimensions, LinearSolver & linearSolver, float dt)
    : TopBoundary(glm::vec2(dimensions.Size.x, 1.0f))
    , BottomBoundary(glm::vec2(dimensions.Size.x, 1.0f))
    , LeftBoundary(glm::vec2(1.0f, dimensions.Size.y))
    , RightBoundary(glm::vec2(1.0f, dimensions.Size.y))
    , mDimensions(dimensions)
    , mData(dimensions.Size)
    , mLinearSolver(linearSolver)
    , mExtrapolation(dimensions)
    , mExtrapolateFluid(Renderer::Shader::TexturePositionVert, ExtrapolateFluidFrag)
    , mConstrainVelocity(Renderer::Shader::TexturePositionVert, ConstrainVelocityFrag)
    , mVelocity(dimensions.Size, 2, true, true)
    , mBoundariesVelocity(dimensions.Size, 2)
    , mFluidLevelSet(dimensions.Size)
    , mObstacleLevelSet(glm::vec2(2.0f)*dimensions.Size)
    , mDiv(Renderer::Shader::TexturePositionVert, DivFrag)
    , mProject(Renderer::Shader::TexturePositionVert, ProjectFrag)
    , mAdvect(Renderer::Shader::TexturePositionVert, AdvectFrag)
    , mWeights(Renderer::Shader::TexturePositionVert, WeightsFrag)
    , mDiagonals(Renderer::Shader::TexturePositionVert, DiagonalsFrag)
    , mFluidProgram(Renderer::Shader::TexturePositionVert, FluidFrag)
    , mColourUniform(mFluidProgram, "u_Colour")
{
    mVelocity.Clear(glm::vec4(0.0));
    mBoundariesVelocity.Clear(glm::vec4(0.0));
    mFluidLevelSet.Clear(glm::vec4(-1.0));
    mObstacleLevelSet.Clear(glm::vec4(-1.0));

    mDiv.Use().Set("u_velocity", 0).Set("u_obstacles", 1).Set("u_obstacles_velocity", 2).Unuse();
    mProject.Use().Set("u_velocity", 0).Set("u_pressure", 1).Set("u_fluid", 2).Set("u_obstacles", 3).Set("u_obstacles_velocity", 4).Set("delta", dt).Unuse();
    mWeights.Use().Set("u_obstacles", 0).Set("u_fluid", 1).Set("delta", dt).Unuse();
    mDiagonals.Use().Set("u_obstacles", 0).Set("u_fluid", 1).Set("delta", dt).Unuse();
    mAdvect.Use().Set("delta", dt).Set("u_texture", 0).Set("u_velocity", 1).Unuse();
    mFluidProgram.Use().Set("u_texture", 0).Unuse();

    mExtrapolateFluid.Use().Set("u_fluid", 0).Set("u_obstacles", 1).Unuse();
    mConstrainVelocity.Use().Set("u_velocity", 0).Set("u_obstacles", 1).Unuse();

    TopBoundary.Colour = BottomBoundary.Colour = LeftBoundary.Colour = RightBoundary.Colour = glm::vec4(1.0f);

    TopBoundary.Position = {0.0f, 0.0f};
    BottomBoundary.Position = glm::vec2(0.0f, dimensions.Size.y - 1.0f);
    LeftBoundary.Position = {0.0f, 0.0f};
    RightBoundary.Position = glm::vec2(dimensions.Size.x - 1.0f, 0.0f);
}

void Engine::Solve()
{
    Renderer::Disable d(GL_BLEND);

    ExtrapolateFluid();

    mData.Pressure.Clear(glm::vec4(0.0));
    mData.Pressure.ClearStencil();
    mObstacleLevelSet.RenderMask(mData.Pressure);
    mFluidLevelSet.RenderMask(mData.Pressure);

    mVelocity.ClearStencil();
    mObstacleLevelSet.RenderMask(mVelocity);
    mFluidLevelSet.RenderMask(mVelocity);

    {
        Renderer::Enable e(GL_STENCIL_TEST);
        glStencilFunc(GL_EQUAL, 0, 0xFF);
        glStencilMask(0x00);

        mData.Pressure = mDiv(mVelocity, mObstacleLevelSet, mBoundariesVelocity);
        mData.Weights = mWeights(mObstacleLevelSet, mFluidLevelSet);
        mData.Diagonal = mDiagonals(mObstacleLevelSet, mFluidLevelSet);

        mLinearSolver.Init(mData);
        mLinearSolver.Solve(mData);

        mVelocity.Swap() = mProject(Back(mVelocity), mData.Pressure, mFluidLevelSet, mObstacleLevelSet, mBoundariesVelocity);
    }

    mExtrapolation.Extrapolate(mVelocity, mObstacleLevelSet, mFluidLevelSet);

    //ConstrainVelocity();

    Advect(mVelocity);
}

void Engine::RenderDirichlet(Renderer::Drawable & object)
{
    Renderer::Disable d(GL_BLEND);
    mFluidLevelSet.Render(object, mDimensions.InvScale);
}

void Engine::RenderNeumann(Renderer::Drawable & object)
{
    Renderer::Disable d(GL_BLEND);
    mObstacleLevelSet.Render(object, glm::scale(glm::vec3(2.0f, 2.0f, 1.0f))*mDimensions.InvScale);
}

void Engine::RenderFluid(Renderer::Drawable &object)
{
    Renderer::Enable d(GL_BLEND);
    Renderer::BlendState b(GL_FUNC_REVERSE_SUBTRACT, GL_ONE, GL_ZERO);

    mFluidLevelSet.Clear(glm::vec4(1.0));
    mFluidLevelSet.Render(object, mDimensions.InvScale);
}

void Engine::RenderVelocities(Renderer::Drawable & object)
{
    Renderer::Disable d(GL_BLEND);
    mBoundariesVelocity.Render(object, mDimensions.InvScale);
}

void Engine::RenderForce(Renderer::Drawable & object)
{
    Renderer::Enable b(GL_BLEND);
    Renderer::BlendState s(GL_FUNC_ADD, GL_ONE, GL_ONE);

    mVelocity.Render(object, mDimensions.InvScale);
}

void Engine::ClearBoundaries()
{
    mObstacleLevelSet.Clear(glm::vec4(-1.0));
    mBoundariesVelocity.Clear(glm::vec4(0.0f));
}

void Engine::ClearVelocities()
{
    mBoundariesVelocity.Clear(glm::vec4(0.0f));
}

void Engine::ReinitialiseDirichlet()
{
    mFluidLevelSet.Redistance(true);
}

void Engine::ReinitialiseNeumann()
{
    mObstacleLevelSet.Redistance(true);
}

void Engine::Advect(Fluid::Buffer & buffer)
{
    Renderer::Disable d(GL_BLEND);
    buffer.Swap() = mAdvect(Back(buffer), Back(mVelocity));
}

void Engine::Render(Renderer::RenderTarget & target, const glm::mat4 & transform)
{
    auto & sprite = mFluidLevelSet.Sprite();
    sprite.SetProgram(mFluidProgram);
    mFluidProgram.Use();
    mColourUniform.Set(Colour);
    target.Render(sprite, glm::scale(glm::vec3(mDimensions.Scale, mDimensions.Scale, 1.0))*transform);
}

void Engine::Advect()
{
    Advect(mFluidLevelSet);
    mFluidLevelSet.Redistance();
}

void Engine::ExtrapolateFluid()
{
    /*
    mFluidLevelSet.ClearStencil();
    mFluidLevelSet.RenderMask(mFluidLevelSet);

    Renderer::Enable e(GL_STENCIL_TEST);
    glStencilFunc(GL_EQUAL, 1, 0xFF);
    glStencilMask(0x00);
    */

    mFluidLevelSet.Swap() = mExtrapolateFluid(Back(mFluidLevelSet), mObstacleLevelSet);
    // FIXME if the obstacles moves, is this correct?

    mFluidLevelSet.Redistance();
}

void Engine::ConstrainVelocity()
{
     mVelocity.ClearStencil();
     mObstacleLevelSet.RenderMask(mVelocity);

     Renderer::Enable e(GL_STENCIL_TEST);
     glStencilFunc(GL_EQUAL, 1, 0xFF);
     glStencilMask(0x00);

     mVelocity.Swap() = mConstrainVelocity(Back(mVelocity), mObstacleLevelSet);
}

}}
