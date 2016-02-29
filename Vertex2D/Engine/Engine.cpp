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
    , mData(dimensions.Size)
    , mBoundaries(boundaries)
    , mAdvection(advection)
    , mLinearSolver(linearSolver)
    , mDiv("TexturePosition.vsh", "Div.fsh")
    , mProject("TexturePosition.vsh", "Project.fsh")
{
    mProject.Use().Set("u_texture", 0).Set("u_pressure", 1).Set("u_weights", 2).Set("u_obstacles_velocity", 3).Unuse();
    mDiv.Use().Set("u_texture", 0).Set("u_weights", 1).Set("u_obstacles_velocity", 2).Unuse();
}

void Engine::Project()
{
    mAdvection.mVelocity.swap();
    mAdvection.mVelocity = mProject(Back(mAdvection.mVelocity), mData.Pressure, mData.Weights, mBoundaries.mBoundariesVelocity);
}

void Engine::Div()
{
    mData.Pressure = mDiv(mAdvection.mVelocity, mData.Weights, mBoundaries.mBoundariesVelocity);
}

void Engine::LinearInit(Boundaries & boundaries)
{
    mLinearSolver->Init(mData, boundaries);
}

void Engine::LinearSolve()
{
    mLinearSolver->Solve(mData);
}

void Engine::Solve()
{
    Renderer::Disable d(GL_BLEND);

    Div();
    mLinearSolver->Init(mData, mBoundaries);
    mLinearSolver->Solve(mData);
    Project();
}

}
