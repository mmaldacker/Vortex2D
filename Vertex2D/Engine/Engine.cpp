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

Engine::Engine(Dimensions dimensions, Boundaries & boundaries, Advection & advection, LinearSolver * linearSolver)
    : mDimensions(dimensions)
    , mQuad(dimensions.Size)
    , mBoundaries(boundaries)
    , mAdvection(advection)
    , mLinearSolver(linearSolver)
{
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
    mAdvection.mVelocity.begin({0.0f, 0.0f, 0.0f, 0.0f});
    mProjectShader.Use().SetMVP(mAdvection.mVelocity.Orth);

    mBoundaries.mBoundariesVelocity.Bind(3);
    mLinearSolver->BindWeights(2);
    mLinearSolver->GetPressure().Front.Bind(1);
    mAdvection.mVelocity.Back.Bind(0);

    mQuad.Render();

    mProjectShader.Unuse();
    mAdvection.mVelocity.end();
}

void Engine::Div()
{
    mBoundaries.mBoundariesVelocity.Bind(2);
    mLinearSolver->BindWeights(1);
    mAdvection.mVelocity.Front.Bind(0);
    mLinearSolver->GetPressure().begin();
    mLinearSolver->Render(mDivShader);
    mLinearSolver->GetPressure().end();
}

void Engine::LinearInit(Boundaries & boundaries)
{
    mLinearSolver->Init(boundaries);
}

void Engine::LinearSolve()
{
    mLinearSolver->Solve();
}

void Engine::Solve()
{
    Renderer::Disable d(GL_BLEND);

    Div();

    mLinearSolver->Solve();

    Project();

    CHECK_GL_ERROR_DEBUG();
}

}
