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
    , mExtrapolate("TexturePosition.vsh", "Extrapolate.fsh")
    , mIdentity("TexturePosition.vsh", "TexturePosition.fsh")
{
    mProject.Use().Set("u_texture", 0).Set("u_pressure", 1).Set("u_obstacles", 2).Set("u_obstacles_velocity", 3).Unuse();
    mDiv.Use().Set("u_texture", 0).Set("u_weights", 1).Set("u_obstacles_velocity", 2).Unuse();
    mExtrapolate.Use().Set("u_texture", 0).Set("u_velocity", 1).Unuse();
}

void Engine::Solve()
{
    Renderer::Enable e(GL_STENCIL_TEST);
    glStencilFunc(GL_EQUAL, 0, 0xFF);
    glStencilMask(0x00);

    mLinearSolver->Init(mData, mBoundaries);
    mData.Pressure = mDiv(mAdvection.mVelocity, mData.Weights, mBoundaries.mBoundariesVelocity);
    mLinearSolver->Solve(mData);
    mAdvection.mVelocity.swap();
    mAdvection.mVelocity = mProject(Back(mAdvection.mVelocity), mData.Pressure, mBoundaries.mNeumannBoundaries, mBoundaries.mBoundariesVelocity);
}

void Engine::Extrapolate(Fluid::LevelSet &levelSet)
{
    Renderer::Enable e(GL_STENCIL_TEST);
    glStencilMask(0x00);
    glStencilFunc(GL_EQUAL, 0, 0xFF);

    mAdvection.mVelocity.swap();
    mAdvection.mVelocity = mIdentity(Back(mAdvection.mVelocity));

    glStencilFunc(GL_EQUAL, 1, 0xFF);
    for(int i = 0 ; i < 10 ; i++)
    {
        mAdvection.mVelocity.swap();
        mAdvection.mVelocity = mExtrapolate(levelSet.mLevelSet, Back(mAdvection.mVelocity));
    }
}

}
