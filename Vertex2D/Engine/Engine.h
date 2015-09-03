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
#include "Size.h"

#include <vector>

namespace Fluid
{

class Engine
{
public:
    Engine(Dimensions dimensions, float dt);
    
    void Solve();
    Renderer::Sprite & GetDensity();
    void Reset();

    void RenderVelocity(const std::vector<Renderer::Drawable*> & objects);
    void RenderDensity(const std::vector<Renderer::Drawable*> & objects);

private:
    Renderer::Program CreateProgramWithShader(const std::string & fragmentSource);

    void Advect(Renderer::PingPong &, Renderer::Program &);
    void Project();
    void Div();

    Dimensions mDimensions;
    Renderer::Quad mQuad;

    Renderer::PingPong mVelocity;
    Renderer::PingPong mDensity;
    Renderer::PingPong mPressure;

    Boundaries mBoundaries;

    SuccessiveOverRelaxation mLinearSolver;
    Renderer::Reader mReader;

    Renderer::Program mAdvectShader;
    Renderer::Program mAdvectDensityShader;
    Renderer::Program mDivShader;
    Renderer::Program mProjectShader;

    float mDt;

    Renderer::Sprite mDensitySprite;
};

}

#endif /* defined(__Vortex__Engine__) */
