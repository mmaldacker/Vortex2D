//
//  GPUFluidEngine.cpp
//  Vortex
//
//  Created by Maximilian Maldacker on 08/04/2014.
//
//

#include "Engine.h"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>
#include <iostream>

namespace Fluid
{

GPUFluidEngine::GPUFluidEngine(const glm::vec2 & size, float scale, float dt, int antiAlias, int iterations)
    : mScale(scale)
    , mAntiAlias(antiAlias)
    , mQuad(glm::floor(size/scale))
    , mVelocity(mQuad.Size().x, mQuad.Size().y, Renderer::Texture::PixelFormat::RGF)
    , mDensity(mQuad.Size().x, mQuad.Size().y, Renderer::Texture::PixelFormat::RGBA8888)
    , mPressure(mQuad.Size().x, mQuad.Size().y, Renderer::Texture::PixelFormat::RGF, Renderer::RenderTexture::DepthFormat::DEPTH24_STENCIL8)
    , mObstacles(mAntiAlias*mQuad.Size().x, mAntiAlias*mQuad.Size().y, Renderer::Texture::PixelFormat::RF)
    , mObstaclesVelocity(mQuad.Size().x, mQuad.Size().y, Renderer::Texture::PixelFormat::RGF)
    , mWeights(mQuad.Size().x, mQuad.Size().y, Renderer::Texture::PixelFormat::RGBAF)
    , mLinearSolver(mQuad, mVelocity, mWeights)
    , mReader(mQuad, mVelocity.Front)
    , mDt(dt)
    , mDensitySprite(mDensity.Front)
    , mRun(true)
    , mHorizontal({mAntiAlias*mQuad.Size().x, mAntiAlias})
    , mVertical({mAntiAlias, mAntiAlias*mQuad.Size().y})
{
    mObstaclesVelocity.SetAliasTexParameters();
    mDensity.Front.SetAntiAliasTexParameters();
    mDensity.Back.SetAntiAliasTexParameters();

    Reset();
    SetupShaders();

    mDensitySprite.SetScale(glm::vec2(mScale));

    mVertical.Colour = {1.0f,1.0f,1.0f,1.0f};
    mHorizontal.Colour = {1.0f,1.0f,1.0f,1.0f};

    CHECK_GL_ERROR_DEBUG();
}

Renderer::Program GPUFluidEngine::CreateProgramWithShader(const std::string & fragmentSource)
{
    Renderer::Program program("fluid.vsh", fragmentSource);
    program.Use().Set("h", mQuad.FullSize());
    return program;
}

void GPUFluidEngine::SetupShaders()
{
    mIdentityShader = &Renderer::Program::TexturePositionProgram();
    mObstacleShader = &Renderer::Program::PositionProgram();

    // obstacles velocity shader
    Renderer::Program obstaclesVelocityProgram("fluid-obstacle-velocity.vsh", "fluid-obstacle-velocity.fsh");
    mObstaclesVelocityShader = std::move(obstaclesVelocityProgram);

    // advection shader
    mAdvectShader = CreateProgramWithShader("fluid-advect.fsh");
    mAdvectShader.Use()
        .Set("delta", mDt)
        .Set("xy_min", glm::vec2{0.5f, 0.5f})
        .Set("xy_max", mQuad.Size() - glm::vec2{1.5})
        .Set("u_texture", 0)
        .Set("u_velocity", 1)
        .Set("u_obstacles", 2);
    
    // advection shader
    mAdvectDensityShader = CreateProgramWithShader("fluid-advect-density.fsh");
    mAdvectDensityShader.Use()
        .Set("delta", mDt)
        .Set("xy_min", glm::vec2{0.5f, 0.5f})
        .Set("xy_max", mQuad.Size() - glm::vec2{1.5})
        .Set("u_texture", 0)
        .Set("u_velocity", 1)
        .Set("u_obstacles", 2);

    // projection shader
    mProjectShader = CreateProgramWithShader("fluid-project.fsh");
    mProjectShader.Use()
        .Set("u_texture", 0)
        .Set("u_pressure", 1)
        .Set("u_weights", 2)
        .Set("u_obstacles_velocity", 3);

    // div shader
    mDivShader = CreateProgramWithShader("fluid-div.fsh");
    mDivShader.Use()
        .Set("u_texture", 0)
        .Set("u_weights", 1)
        .Set("u_obstacles_velocity", 2);

    // weights shader
    mWeightsShader = CreateProgramWithShader("fluid-weights.fsh");
    mWeightsShader.Use()
        .Set("u_texture", 0);
}

void GPUFluidEngine::Sources()
{
    mObstacles.begin({0.0f, 0.0f, 0.0f, 0.0f});
    mHorizontal.SetPosition({0.0f, 0.0f});
    mHorizontal.Render(mObstacles.Orth);

    mHorizontal.SetPosition({0.0f, mAntiAlias*(mQuad.Size().y-1.0f)});
    mHorizontal.Render(mObstacles.Orth);

    mVertical.SetPosition({0.0f, 0.0f});
    mVertical.Render(mObstacles.Orth);

    mVertical.SetPosition({mAntiAlias*(mQuad.Size().x-1.0f), 0.0f});
    mVertical.Render(mObstacles.Orth);

    auto invScale = glm::scale(glm::vec3{1.0f/mScale, 1.0f/mScale, 1.0f});
    invScale = glm::translate(invScale, glm::vec3{-mScale*0.5f, -mScale*0.5f, 0.0f});

    auto scaled = glm::scale(mObstacles.Orth, glm::vec3(mAntiAlias, mAntiAlias, 1.0f));
    for(auto input : mFluidInputs)
    {
        input->RenderObstacle(scaled*invScale);
    }
    mObstacles.end();

    mObstaclesVelocity.begin({0.0f, 0.0f, 0.0f, 0.0f});
    for(auto input : mFluidInputs)
    {
        input->RenderObstacleVelocity(mObstaclesVelocity.Orth*invScale);
    }
    mObstaclesVelocity.end();

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

void GPUFluidEngine::Weights()
{
    mWeights.begin();
    mWeightsShader.Use().SetMVP(mWeights.Orth);

    mObstacles.Bind(0);
    mQuad.Render();

    mWeights.end();
}

void GPUFluidEngine::Advect(Renderer::PingPong & renderTexture, Renderer::Program & program)
{
    renderTexture.swap();
    renderTexture.begin();
    program.Use().SetMVP(renderTexture.Orth);

    mObstacles.Bind(2);
    mVelocity.Back.Bind(1);
    renderTexture.Back.Bind(0);

    mQuad.Render();

    renderTexture.end();
}

void GPUFluidEngine::Project()
{
    mVelocity.swap();
    mVelocity.begin();
    mProjectShader.Use().SetMVP(mVelocity.Orth);

    mObstaclesVelocity.Bind(3);
    mWeights.Bind(2);
    mPressure.Front.Bind(1);
    mVelocity.Back.Bind(0);

    mQuad.Render();

    mVelocity.end();
}

void GPUFluidEngine::Div()
{
    mPressure.begin();
    mDivShader.Use().SetMVP(mPressure.Orth);

    mObstaclesVelocity.Bind(2);
    mWeights.Bind(1);
    mVelocity.Front.Bind(0);

    mQuad.Render();

    mPressure.end();
}

void GPUFluidEngine::Solve()
{
    if(!mRun) return;

    GLint blend = GL_FALSE;
    glGetIntegerv(GL_BLEND, &blend);
    if(blend == GL_TRUE)
    {
        glDisable(GL_BLEND);
    }

    mReader.Read();

    Sources();
    Weights();
    Div();

    mLinearSolver.Solve();

    Project();
    Advect(mVelocity, mAdvectShader);
    Advect(mDensity, mAdvectDensityShader);

    if(blend == GL_TRUE)
    {
        glEnable(GL_BLEND);
    }

    CHECK_GL_ERROR_DEBUG();
}

void GPUFluidEngine::AddInput(FluidInput *input)
{
    mFluidInputs.push_back(input);
}

Renderer::Sprite & GPUFluidEngine::GetDensity()
{
    return mDensitySprite;
}

void GPUFluidEngine::Reset()
{
    mObstacles.Clear();
    mObstaclesVelocity.Clear();
    mWeights.Clear();
    mVelocity.Clear();
    mDensity.Clear();
    mPressure.Clear();
    mFluidInputs.clear();
}

void GPUFluidEngine::Start()
{
    mRun = true;
}

void GPUFluidEngine::Stop()
{
    mRun = false;
}

}
