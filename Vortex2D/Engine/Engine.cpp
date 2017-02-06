//
//  Engine.cpp
//  Vortex
//

#include "Engine.h"
#include "Disable.h"

namespace Vortex2D { namespace Fluid {

namespace
{


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
    //, mExtrapolation(dimensions)
    , mVelocity(dimensions.Size, 2, true, true)
    , mBoundariesVelocity(dimensions.Size, 2)
    , mFluidLevelSet(dimensions.Size)
    , mObstacleLevelSet(glm::vec2(2.0f)*dimensions.Size)
    , mFluidProgram(Renderer::Shader::TexturePositionVert, FluidFrag)
    , mColourUniform(mFluidProgram, "u_Colour")
{
    mVelocity.Clear(glm::vec4(0.0));
    mBoundariesVelocity.Clear(glm::vec4(0.0));
    mFluidLevelSet.Clear(glm::vec4(-1.0));
    mObstacleLevelSet.Clear(glm::vec4(-1.0));


    mFluidProgram.Use().Set("u_texture", 0).Unuse();


    TopBoundary.Colour = BottomBoundary.Colour = LeftBoundary.Colour = RightBoundary.Colour = glm::vec4(1.0f);

    TopBoundary.Position = {0.0f, 0.0f};
    BottomBoundary.Position = glm::vec2(dimensions.Scale)*glm::vec2(0.0f, dimensions.Size.y - 1.0f);
    LeftBoundary.Position = {0.0f, 0.0f};
    RightBoundary.Position = glm::vec2(dimensions.Scale)*glm::vec2(dimensions.Size.x - 1.0f, 0.0f);
}

void Engine::Solve()
{
    Renderer::Disable d(GL_BLEND);

    // FIXME extrapolate liquid phi
    //Extrapolate();

    mData.Pressure.Clear(glm::vec4(0.0));
    mData.Pressure.ClearStencil();
    //mObstacleLevelSet.RenderMask(mData.Pressure);
    //mFluidLevelSet.RenderMask(mData.Pressure);

    mVelocity.ClearStencil();
    //mObstacleLevelSet.RenderMask(mVelocity);
    //mFluidLevelSet.RenderMask(mVelocity);

    // FIXME use pressure solver

    // FIXME extrapolation
    //mExtrapolation.Extrapolate(mVelocity, mObstacleLevelSet, mFluidLevelSet);

    // FIXME advect
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
    //FIXME
    //Advect(mFluidLevelSet);
    mFluidLevelSet.Redistance(2);
}

}}
