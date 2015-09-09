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

Engine::Engine(Dimensions dimensions, Boundaries & boundaries, Advection & advection)
    : mDimensions(dimensions)
    , mQuad(dimensions.Size)
    , mPressure(dimensions.Size.x, dimensions.Size.y, Renderer::Texture::PixelFormat::RGF, Renderer::RenderTexture::DepthFormat::DEPTH24_STENCIL8)
    , mBoundaries(boundaries)
    , mAdvection(advection)
    , mLinearSolver(dimensions, mBoundaries.mWeights, mPressure, mBoundaries)
{
    mPressure.Clear();

    // projection shader
    mProjectShader = Renderer::Program("Diff.vsh", "Project.fsh");
    mProjectShader.Use()
    .Set("u_texture", 0)
    .Set("u_pressure", 1)
    .Set("u_weights", 2)
    .Set("u_obstacles_velocity", 3)
    .Set("h", mQuad.Size())
    .Unuse();

    // div shader
    mDivShader = Renderer::Program("Diff.vsh", "Div.fsh");
    mDivShader.Use()
    .Set("u_texture", 0)
    .Set("u_weights", 1)
    .Set("u_obstacles_velocity", 2)
    .Set("h", mQuad.Size())
    .Unuse();
}

void Engine::Project()
{
    mAdvection.mVelocity.swap();
    mAdvection.mVelocity.begin();
    mProjectShader.Use().SetMVP(mAdvection.mVelocity.Orth);

    mBoundaries.mBoundaries.Bind(3);
    mBoundaries.mWeights.Bind(2);
    mPressure.Front.Bind(1);
    mAdvection.mVelocity.Back.Bind(0);

    mQuad.Render();

    mProjectShader.Unuse();
    mAdvection.mVelocity.end();
}

void Engine::Div()
{
    mPressure.begin({0.0f, 0.0f, 0.0f, 0.0f});
    mDivShader.Use().SetMVP(mPressure.Orth);

    mBoundaries.mBoundariesVelocity.Bind(2);
    mBoundaries.mBoundaries.Bind(1);
    mAdvection.mVelocity.Front.Bind(0);

    mQuad.Render();

    mDivShader.Unuse();
    mPressure.end();
}

void Engine::LinearInit(const std::vector<Renderer::Drawable*> & objects)
{
    mLinearSolver.RenderMask(objects);
}

void Engine::LinearSolve()
{
    mLinearSolver.Solve();
}

void Engine::Solve()
{
    Renderer::Disable d(GL_BLEND);

    Div();

    mLinearSolver.Solve();

    Project();

    CHECK_GL_ERROR_DEBUG();
}

Renderer::Reader Engine::GetPressureReader()
{
    return {mPressure.Front};
}

}
