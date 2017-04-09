//
//  World.cpp
//  Vortex2D
//

#include "World.h"

#include <Vortex2D/Renderer/Disable.h>

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

World::World(Dimensions dimensions, float dt)
    : mDimensions(dimensions)
    , mData(dimensions.Size)
    , mLinearSolver(dimensions.Size)
    , mVelocity(dimensions.Size, 2, true)
    , mBoundariesVelocity(dimensions.Size, 2)
    , mFluidLevelSet(dimensions.Size)
    , mObstacleLevelSet(glm::vec2(2.0f)*dimensions.Size)
    , mAdvection(dt, mVelocity)
    , mPressure(dt, dimensions.Size, mLinearSolver, mData, mVelocity, mObstacleLevelSet, mFluidLevelSet, mBoundariesVelocity)
    , mExtrapolation(dimensions.Size, mVelocity, mObstacleLevelSet)
    , mVelocityReader(mVelocity)
    , mFluidProgram(Renderer::Shader::TexturePositionVert, FluidFrag)
    , mColourUniform(mFluidProgram, "u_Colour")
{
    mVelocity.Clear(glm::vec4(0.0));
    mBoundariesVelocity.Clear(glm::vec4(0.0));

    mFluidProgram.Use().Set("u_texture", 0).Unuse();
}

Boundaries World::DrawBoundaries()
{
    return Boundaries(mDimensions, mFluidLevelSet, mObstacleLevelSet);
}

void World::Solve()
{
    Renderer::Disable d(GL_BLEND);

    mFluidLevelSet.Extrapolate(mObstacleLevelSet);

    LinearSolver::Parameters params(300, 1e-5f);
    mPressure.Solve(params);

    mFluidLevelSet.Swap();

    mExtrapolation.Extrapolate();
    mExtrapolation.ConstrainVelocity();

    mAdvection.Advect();
}

void World::RenderForce(Renderer::Drawable& object)
{
    Renderer::Enable b(GL_BLEND);
    Renderer::BlendState s(GL_FUNC_ADD, GL_ONE, GL_ONE);

    mVelocity.Render(object, mDimensions.InvScale);
}

void World::Render(Renderer::RenderTarget& target, const glm::mat4& transform)
{
    auto & sprite = mFluidLevelSet.Sprite();
    sprite.SetProgram(mFluidProgram);
    mFluidProgram.Use();
    mColourUniform.Set(Colour);
    target.Render(sprite, glm::scale(glm::vec3(mDimensions.Scale, mDimensions.Scale, 1.0))*transform);
}

void World::Advect()
{
    Renderer::Disable d(GL_BLEND);

    mAdvection.Advect(mFluidLevelSet);
    mFluidLevelSet.Redistance(2);
}

void World::Advect(Renderer::Buffer& buffer)
{
    Renderer::Disable d(GL_BLEND);

    mAdvection.Advect(buffer);
}

Renderer::Reader& World::GetVelocityReader()
{
    mVelocityReader.Read();
    return mVelocityReader;
}

}}
