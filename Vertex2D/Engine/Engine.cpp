//
//  Engine.cpp
//  Vortex
//
//  Created by Maximilian Maldacker on 08/04/2014.
//
//

#include "Engine.h"
#include "Disable.h"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>
#include <iostream>

namespace Fluid
{

Engine::Engine(Dimensions dimensions, float dt)
    : mDimensions(dimensions)
    , mQuad(dimensions.Size)
    , mVelocity(dimensions.Size.x, dimensions.Size.y, Renderer::Texture::PixelFormat::RGF)
    , mDensity(dimensions.Size.x, dimensions.Size.y, Renderer::Texture::PixelFormat::RGBA8888)
    , mPressure(dimensions.Size.x, dimensions.Size.y, Renderer::Texture::PixelFormat::RGF, Renderer::RenderTexture::DepthFormat::DEPTH24_STENCIL8)
    , mBoundaries(dimensions, 2)
    , mLinearSolver(mQuad, mVelocity, mBoundaries)
    , mDt(dt)
    , mDensitySprite(mDensity.Front)
{
    mDensity.Front.SetAntiAliasTexParameters();
    mDensity.Back.SetAntiAliasTexParameters();
    mDensitySprite.Scale = glm::vec2(dimensions.Scale);

    Reset();

    // advection shader
    mAdvectShader = CreateProgramWithShader("Advect.fsh");
    mAdvectShader.Use()
    .Set("delta", mDt)
    .Set("xy_min", glm::vec2{0.5f, 0.5f})
    .Set("xy_max", dimensions.Size - glm::vec2{1.5})
    .Set("u_texture", 0)
    .Set("u_velocity", 1)
    .Set("u_obstacles", 2)
    .Unuse();

    // advection shader
    mAdvectDensityShader = CreateProgramWithShader("AdvectDensity.fsh");
    mAdvectDensityShader.Use()
    .Set("delta", mDt)
    .Set("xy_min", glm::vec2{0.5f, 0.5f})
    .Set("xy_max", mDimensions.Size - glm::vec2{1.5})
    .Set("u_texture", 0)
    .Set("u_velocity", 1)
    .Set("u_obstacles", 2)
    .Unuse();

    // projection shader
    mProjectShader = CreateProgramWithShader("Project.fsh");
    mProjectShader.Use()
    .Set("u_texture", 0)
    .Set("u_pressure", 1)
    .Set("u_weights", 2)
    .Set("u_obstacles_velocity", 3)
    .Unuse();

    // div shader
    mDivShader = CreateProgramWithShader("Div.fsh");
    mDivShader.Use()
    .Set("u_texture", 0)
    .Set("u_weights", 1)
    .Set("u_obstacles_velocity", 2)
    .Unuse();
}

Renderer::Program Engine::CreateProgramWithShader(const std::string & fragmentSource)
{
    Renderer::Program program("Diff.vsh", fragmentSource);
    program.Use().Set("h", mQuad.Size());
    return program;
}

void Engine::RenderVelocity(const std::vector<Renderer::Drawable*> & objects)
{
    mVelocity.begin();
    for(auto object : objects)
    {
        object->Render(mVelocity.Orth*mDimensions.InvScale);
    }
    mVelocity.end();
}

void Engine::RenderDensity(const std::vector<Renderer::Drawable*> & objects)
{
    mDensity.begin();
    for(auto object : objects)
    {
        object->Render(mDensity.Orth*mDimensions.InvScale);
    }
    mDensity.end();
}

void Engine::Advect(Renderer::PingPong & renderTexture, Renderer::Program & program)
{
    renderTexture.swap();
    renderTexture.begin();
    program.Use().SetMVP(renderTexture.Orth);

    //mBoundaries.Bind(2);
    mVelocity.Back.Bind(1);
    renderTexture.Back.Bind(0);

    mQuad.Render();

    program.Unuse();
    renderTexture.end();
}

void Engine::Project()
{
    mVelocity.swap();
    mVelocity.begin();
    mProjectShader.Use().SetMVP(mVelocity.Orth);

    //mBoundariesVelocity.Bind(3);
    //mWeights.Bind(2);
    mPressure.Front.Bind(1);
    mVelocity.Back.Bind(0);

    mQuad.Render();

    mProjectShader.Unuse();
    mVelocity.end();
}

void Engine::Div()
{
    mPressure.begin();
    mDivShader.Use().SetMVP(mPressure.Orth);

    //mBoundariesVelocity.Bind(2);
    //mWeights.Bind(1);
    mVelocity.Front.Bind(0);

    mQuad.Render();

    mDivShader.Unuse();
    mPressure.end();
}

void Engine::Solve()
{
    Renderer::Disable d(GL_BLEND);

    Div();

    mLinearSolver.Solve();

    Project();

    Advect(mVelocity, mAdvectShader);
    Advect(mDensity, mAdvectDensityShader);

    CHECK_GL_ERROR_DEBUG();
}

Renderer::Sprite & Engine::GetDensity()
{
    return mDensitySprite;
}

void Engine::Reset()
{
    mBoundaries.Clear();
    mVelocity.Clear();
    mDensity.Clear();
    mPressure.Clear();
}

}
