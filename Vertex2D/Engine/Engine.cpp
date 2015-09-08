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
    , mLinearSolver(mQuad, mPressure, mBoundaries, 1)
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
/*
    mVelocity.swap();
    mVelocity.begin();
    mProjectShader.Use().SetMVP(mVelocity.Orth);

    mBoundaries.BindBoundary(3);
    mBoundaries.BindWeights(2);
    mPressure.Front.Bind(1);
    mVelocity.Back.Bind(0);

    mQuad.Render();

    mProjectShader.Unuse();
    mVelocity.end();
*/
}

void Engine::Div()
{
/*
    mPressure.begin();
    mDivShader.Use().SetMVP(mPressure.Orth);

    mBoundaries.BindVelocity(2);
    mBoundaries.BindBoundary(1);
    mVelocity.Front.Bind(0);

    mQuad.Render();

    mDivShader.Unuse();
    mPressure.end();
*/
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
