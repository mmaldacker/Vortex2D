//
//  Engine.h
//  Vortex
//
//  Created by Maximilian Maldacker on 08/04/2014.
//
//

#ifndef __Vortex__Engine__
#define __Vortex__Engine__

#include "SuccessiveOverRelaxation.h"
#include "Shader.h"
#include "RenderTexture.h"
#include "Sprite.h"
#include "Shapes.h"
#include "Reader.h"
#include "Boundaries.h"
#include "Advection.h"
#include "Size.h"

#include <vector>

namespace Fluid
{

class Engine
{
public:
    Engine(Dimensions dimensions, Boundaries & boundaries, Advection & advection);
    
    void Solve();

    Renderer::Reader GetPressureReader();

    void Div();
    void Project();

    void LinearInit(Boundaries & boundaries);
    void LinearSolve();

private:
    Dimensions mDimensions;
    Renderer::Quad mQuad;

    Renderer::PingPong mPressure;

    Boundaries & mBoundaries;
    Advection & mAdvection;

    SuccessiveOverRelaxation mLinearSolver;

    Renderer::Program mDivShader;
    Renderer::Program mProjectShader;
};

}

#endif /* defined(__Vortex__Engine__) */
