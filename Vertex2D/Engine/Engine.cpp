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

Engine::Engine(const glm::vec2 & size, float scale, float dt, int antiAlias, int iterations)
    : mScale(scale)
    , mAntiAlias(antiAlias)
    , mSize(glm::floor(size/scale))
    , mQuad(mSize)
    , mVelocity(mSize.x, mSize.y, Renderer::Texture::PixelFormat::RGF)
    , mDensity(mSize.x, mSize.y, Renderer::Texture::PixelFormat::RGBA8888)
    , mPressure(mSize.x, mSize.y, Renderer::Texture::PixelFormat::RGF, Renderer::RenderTexture::DepthFormat::DEPTH24_STENCIL8)
    , mBoundaries(mAntiAlias*mSize.x, mAntiAlias*mSize.y, Renderer::Texture::PixelFormat::RF)
    , mBoundariesVelocity(mSize.x, mSize.y, Renderer::Texture::PixelFormat::RGF)
    , mWeights(mSize.x, mSize.y, Renderer::Texture::PixelFormat::RGBAF)
    , mLinearSolver(mQuad, mVelocity, mWeights)
    , mReader(mQuad, mVelocity.Front)
    , mDt(dt)
    , mDensitySprite(mDensity.Front)
    , mHorizontal({mAntiAlias*mSize.x, mAntiAlias})
    , mVertical({mAntiAlias, mAntiAlias*mSize.y})
{
    mBoundariesVelocity.SetAliasTexParameters();
    mDensity.Front.SetAntiAliasTexParameters();
    mDensity.Back.SetAntiAliasTexParameters();

    Reset();
    SetupShaders();

    mDensitySprite.Scale = glm::vec2(mScale);

    mVertical.Colour = {1.0f,1.0f,1.0f,1.0f};
    mHorizontal.Colour = {1.0f,1.0f,1.0f,1.0f};

    CHECK_GL_ERROR_DEBUG();
}

Renderer::Program Engine::CreateProgramWithShader(const std::string & fragmentSource)
{
    Renderer::Program program("Diff.vsh", fragmentSource);
    program.Use().Set("h", mQuad.Size());
    return program;
}

void Engine::SetupShaders()
{
    mIdentityShader = &Renderer::Program::TexturePositionProgram();
    mBoundarieshader = &Renderer::Program::PositionProgram();

    // obstacles velocity shader
    Renderer::Program obstaclesVelocityProgram("fluid-obstacle-velocity.vsh", "fluid-obstacle-velocity.fsh");
    mBoundariesVelocityShader = std::move(obstaclesVelocityProgram);

    // advection shader
    mAdvectShader = CreateProgramWithShader("Advect.fsh");
    mAdvectShader.Use()
        .Set("delta", mDt)
        .Set("xy_min", glm::vec2{0.5f, 0.5f})
        .Set("xy_max", mSize - glm::vec2{1.5})
        .Set("u_texture", 0)
        .Set("u_velocity", 1)
        .Set("u_obstacles", 2)
        .Unuse();
    
    // advection shader
    mAdvectDensityShader = CreateProgramWithShader("AdvectDensity.fsh");
    mAdvectDensityShader.Use()
        .Set("delta", mDt)
        .Set("xy_min", glm::vec2{0.5f, 0.5f})
        .Set("xy_max", mSize - glm::vec2{1.5})
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

    // weights shader
    mWeightsShader = CreateProgramWithShader("Weights.fsh");
    mWeightsShader.Use()
        .Set("u_texture", 0)
        .Unuse();
}

void Engine::Sources()
{
    mBoundaries.begin({0.0f, 0.0f, 0.0f, 0.0f});
    mHorizontal.Position = {0.0f, 0.0f};
    mHorizontal.Render(mBoundaries.Orth);

    mHorizontal.Position = {0.0f, mAntiAlias*(mSize.y-1.0f)};
    mHorizontal.Render(mBoundaries.Orth);

    mVertical.Position = {0.0f, 0.0f};
    mVertical.Render(mBoundaries.Orth);

    mVertical.Position = {mAntiAlias*(mSize.x-1.0f), 0.0f};
    mVertical.Render(mBoundaries.Orth);

    auto invScale = glm::scale(glm::vec3{1.0f/mScale, 1.0f/mScale, 1.0f});
    invScale = glm::translate(invScale, glm::vec3{-mScale*0.5f, -mScale*0.5f, 0.0f});

    auto scaled = glm::scale(mBoundaries.Orth, glm::vec3(mAntiAlias, mAntiAlias, 1.0f));
    for(auto input : mFluidInputs)
    {
        input->RenderObstacle(scaled*invScale);
    }
    mBoundaries.end();

    mBoundariesVelocity.begin({0.0f, 0.0f, 0.0f, 0.0f});
    for(auto input : mFluidInputs)
    {
        input->RenderObstacleVelocity(mBoundariesVelocity.Orth*invScale);
    }
    mBoundariesVelocity.end();

    mVelocity.begin();
    for(auto input : mFluidInputs)
    {
        input->RenderVelocity(mVelocity.Orth*invScale);
    }
    mVelocity.end();

    mDensity.begin();
    for(auto input : mFluidInputs)
    {
        input->RenderDensity(mDensity.Orth*invScale);
    }
    mDensity.end();
}

void Engine::Weights()
{
    mWeights.begin();
    mWeightsShader.Use().SetMVP(mWeights.Orth);

    mBoundaries.Bind(0);
    mQuad.Render();

    mWeightsShader.Unuse();
    mWeights.end();
}

void Engine::Advect(Renderer::PingPong & renderTexture, Renderer::Program & program)
{
    renderTexture.swap();
    renderTexture.begin();
    program.Use().SetMVP(renderTexture.Orth);

    mBoundaries.Bind(2);
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

    mBoundariesVelocity.Bind(3);
    mWeights.Bind(2);
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

    mBoundariesVelocity.Bind(2);
    mWeights.Bind(1);
    mVelocity.Front.Bind(0);

    mQuad.Render();

    mDivShader.Unuse();
    mPressure.end();
}

void Engine::Solve()
{
    Renderer::Disable d(GL_BLEND);

    mReader.Read();

    Sources();
    Weights();
    Div();

    mLinearSolver.Solve();

    Project();
    Advect(mVelocity, mAdvectShader);
    Advect(mDensity, mAdvectDensityShader);

    CHECK_GL_ERROR_DEBUG();
}

void Engine::AddInput(FluidInput *input)
{
    mFluidInputs.push_back(input);
}

Renderer::Sprite & Engine::GetDensity()
{
    return mDensitySprite;
}

void Engine::Reset()
{
    mBoundaries.Clear();
    mBoundariesVelocity.Clear();
    mWeights.Clear();
    mVelocity.Clear();
    mDensity.Clear();
    mPressure.Clear();
    mFluidInputs.clear();
}

}
