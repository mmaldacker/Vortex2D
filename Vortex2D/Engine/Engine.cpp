//
//  Engine.cpp
//  Vortex
//

#include "Engine.h"
#include "Disable.h"

namespace Vortex2D { namespace Fluid {

namespace
{

const char * AdvectVelocityFrag = GLSL(
    uniform sampler2D u_velocity;
    uniform float delta;

    in vec2 v_texCoord;
    out vec4 out_color;

    vec2 cubic(vec2 f1, vec2 f2, vec2 f3, vec2 f4, float xd)
    {
        float xd2 = xd * xd;
        float xd3 = xd2 * xd;

        return f1*(     - 0.5*xd  +     xd2 - 0.5*xd3) +
               f2*( 1.0           - 2.5*xd2 + 1.5*xd3) +
               f3*(       0.5*xd  + 2.0*xd2 - 1.5*xd3) +
               f4*(               - 0.5*xd2 + 0.5*xd3);
    }

    vec2 bicubic(vec2 xy)
    {
        ivec2 ij = ivec2(floor(xy)) - 1;
        vec2 f = xy - (ij + 1);

        vec2 t[16];
        for(int j = 0 ; j < 4 ; ++j)
        {
            for(int i = 0 ; i < 4 ; ++i)
            {
                t[i + 4*j] = texelFetch(u_velocity, ij + ivec2(i,j), 0).xy;
            }
        }

        vec2 x = cubic(
                    cubic(t[0], t[4], t[8], t[12], f.y),
                    cubic(t[1], t[5], t[9], t[13], f.y),
                    cubic(t[2], t[6], t[10], t[14], f.y),
                    cubic(t[3], t[7], t[11], t[15], f.y),
                    f.x
                );

        vec2 maxValue = max(max(t[5], t[6]), max(t[9], t[10]));
        vec2 minValue = min(min(t[5], t[6]), min(t[9], t[10]));

        return clamp(x, minValue, maxValue);
    }

    const float a = 2.0/9.0;
    const float b = 3.0/9.0;
    const float c = 4.0/9.0;

    void main(void)
    {
        vec2 pos = gl_FragCoord.xy - 0.5;

        vec2 k1 = texture(u_velocity, v_texCoord).xy;
        vec2 k2 = bicubic(pos - 0.5*delta*k1);
        vec2 k3 = bicubic(pos - 0.75*delta*k2);

        out_color = vec4(bicubic(pos - a*delta*k1 - b*delta*k2 - c*delta*k3), 0.0, 0.0);
    }
);

const char * AdvectFrag = GLSL(
    uniform sampler2D u_texture;
    uniform sampler2D u_velocity;
    uniform float delta;

    in vec2 v_texCoord;
    out vec4 out_color;

    vec2 get_velocity(ivec2 pos)
    {
       vec2 uv = texelFetch(u_velocity, pos, 0).xy;
       float up = texelFetch(u_velocity, pos + ivec2(1,0), 0).x;
       float vp = texelFetch(u_velocity, pos + ivec2(0,1), 0).y;

       return vec2(uv.x + up, uv.y + vp) * 0.5;
    }

    vec4[16] get_velocity_samples(ivec2 ij)
    {
       vec4 t[16];
       for(int j = 0 ; j < 4 ; ++j)
       {
           for(int i = 0 ; i < 4 ; ++i)
           {
               t[i + 4*j] = vec4(get_velocity(ij + ivec2(i,j)), 0.0, 0.0);
           }
       }
       return t;
    }

    vec4[16] get_samples(ivec2 ij)
    {
       vec4 t[16];
       for(int j = 0 ; j < 4 ; ++j)
       {
           for(int i = 0 ; i < 4 ; ++i)
           {
               t[i + 4*j] = texelFetch(u_texture, ij + ivec2(i,j), 0);
           }
       }
       return t;
    }

    vec4 cubic(vec4 f1, vec4 f2, vec4 f3, vec4 f4, float xd)
    {
       float xd2 = xd * xd;
       float xd3 = xd2 * xd;

       return f1*(     - 0.5*xd  +     xd2 - 0.5*xd3) +
       f2*( 1.0           - 2.5*xd2 + 1.5*xd3) +
       f3*(       0.5*xd  + 2.0*xd2 - 1.5*xd3) +
       f4*(               - 0.5*xd2 + 0.5*xd3);
    }

    vec4 bicubic(vec4 t[16], vec2 f)
    {

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

    vec2 interpolate_velocity(vec2 xy)
    {
       ivec2 ij = ivec2(floor(xy)) - 1;
       vec2 f = xy - (ij + 1);

       return bicubic(get_velocity_samples(ij), f).xy;
    }

    vec4 interpolate(vec2 xy)
    {
       ivec2 ij = ivec2(floor(xy)) - 1;
       vec2 f = xy - (ij + 1);

       return bicubic(get_samples(ij), f);
    }

    const float a = 2.0/9.0;
    const float b = 3.0/9.0;
    const float c = 4.0/9.0;

    void main(void)
    {
       vec2 pos = gl_FragCoord.xy - 0.5;

       vec2 k1 = get_velocity(ivec2(pos));
       vec2 k2 = interpolate_velocity(pos - 0.5*delta*k1);
       vec2 k3 = interpolate_velocity(pos - 0.75*delta*k2);

       out_color = interpolate(pos - a*delta*k1 - b*delta*k2 - c*delta*k3);
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
         if(f <= 1.0 && f > 0.0 && o >= 0.0)
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

}

using Renderer::Back;

Engine::Engine(Dimensions dimensions, LinearSolver& linearSolver, float dt)
    : TopBoundary(glm::vec2(dimensions.Scale)*glm::vec2(dimensions.Size.x, 1.0f))
    , BottomBoundary(glm::vec2(dimensions.Scale)*glm::vec2(dimensions.Size.x, 1.0f))
    , LeftBoundary(glm::vec2(dimensions.Scale)*glm::vec2(1.0f, dimensions.Size.y))
    , RightBoundary(glm::vec2(dimensions.Scale)*glm::vec2(1.0f, dimensions.Size.y))
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

    , mVelocityAdvect(Renderer::Shader::TexturePositionVert, AdvectVelocityFrag)
    , mAdvect(Renderer::Shader::TexturePositionVert, AdvectFrag)

    , mFluidProgram(Renderer::Shader::TexturePositionVert, FluidFrag)
    , mColourUniform(mFluidProgram, "u_Colour")
{
    mVelocity.Clear(glm::vec4(0.0));
    mBoundariesVelocity.Clear(glm::vec4(0.0));
    mFluidLevelSet.Clear(glm::vec4(-1.0));
    mObstacleLevelSet.Clear(glm::vec4(-1.0));

    mVelocityAdvect.Use().Set("delta", dt).Set("u_velocity", 0).Unuse();
    mAdvect.Use().Set("delta", dt).Set("u_texture", 0).Set("u_velocity", 1).Unuse();
    mFluidProgram.Use().Set("u_texture", 0).Unuse();

    mExtrapolateFluid.Use().Set("u_fluid", 0).Set("u_obstacles", 1).Unuse();
    mConstrainVelocity.Use().Set("u_velocity", 0).Set("u_obstacles", 1).Unuse();

    TopBoundary.Colour = BottomBoundary.Colour = LeftBoundary.Colour = RightBoundary.Colour = glm::vec4(1.0f);

    TopBoundary.Position = {0.0f, 0.0f};
    BottomBoundary.Position = glm::vec2(dimensions.Scale)*glm::vec2(0.0f, dimensions.Size.y - 1.0f);
    LeftBoundary.Position = {0.0f, 0.0f};
    RightBoundary.Position = glm::vec2(dimensions.Scale)*glm::vec2(dimensions.Size.x - 1.0f, 0.0f);
}

void Engine::Solve()
{
    Renderer::Disable d(GL_BLEND);

    ExtrapolateFluid();

    mData.Pressure.Clear(glm::vec4(0.0));
    mData.Pressure.ClearStencil();
    //mObstacleLevelSet.RenderMask(mData.Pressure);
    //mFluidLevelSet.RenderMask(mData.Pressure);

    mVelocity.ClearStencil();
    //mObstacleLevelSet.RenderMask(mVelocity);
    //mFluidLevelSet.RenderMask(mVelocity);

    // FIXME use pressure solver

    mExtrapolation.Extrapolate(mVelocity, mObstacleLevelSet, mFluidLevelSet);

    //ConstrainVelocity();

    mVelocity.Swap();
    mVelocity = mVelocityAdvect(Back(mVelocity));
}

void Engine::RenderDirichlet(Renderer::Drawable& object)
{
    Renderer::Disable d(GL_BLEND);
    mFluidLevelSet.Render(object, mDimensions.InvScale);
}

void Engine::RenderNeumann(Renderer::Drawable& object)
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

void Engine::RenderVelocities(Renderer::Drawable& object)
{
    Renderer::Disable d(GL_BLEND);
    mBoundariesVelocity.Render(object, mDimensions.InvScale);
}

void Engine::RenderForce(Renderer::Drawable& object)
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
    mFluidLevelSet.Redistance(100);
}

void Engine::ReinitialiseNeumann()
{
    mObstacleLevelSet.Redistance(100);
}

void Engine::Advect(Renderer::Buffer& buffer)
{
    Renderer::Disable d(GL_BLEND);
    buffer.Swap();
    buffer = mAdvect(Back(buffer), mVelocity);
}

void Engine::Render(Renderer::RenderTarget& target, const glm::mat4& transform)
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
    mFluidLevelSet.Redistance(2);
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

    mFluidLevelSet.Swap();
    mFluidLevelSet = mExtrapolateFluid(Back(mFluidLevelSet), mObstacleLevelSet);
    // FIXME if the obstacles moves, is this correct?

    mFluidLevelSet.Redistance(2);
}

void Engine::ConstrainVelocity()
{
     mVelocity.ClearStencil();
     //mObstacleLevelSet.RenderMask(mVelocity);

     Renderer::Enable e(GL_STENCIL_TEST);
     glStencilFunc(GL_EQUAL, 1, 0xFF);
     glStencilMask(0x00);

     mVelocity.Swap();
     mVelocity = mConstrainVelocity(Back(mVelocity), mObstacleLevelSet);
}

}}
