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
#include "FluidInput.h"
#include "Sprite.h"
#include "Shapes.h"
#include "Reader.h"
#include "hfloat.h"

#include <vector>

namespace Fluid
{

class Engine
{
public:
    Engine(const glm::vec2 & size, float scale, float dt, int antiAlias, int iterations);
    
    void Solve();
    void AddInput(FluidInput * input);
    Renderer::Sprite & GetDensity();
    void Reset();

private:
    Renderer::Program CreateProgramWithShader(const std::string & fragmentSource);

    void SetupShaders();

    void Sources();
    void Weights();
    void Advect(Renderer::PingPong &, Renderer::Program &);
    void Project();
    void Div();

    float mScale;
    int mAntiAlias;

    glm::vec2 mSize;
    Renderer::Quad mQuad;

    Renderer::PingPong mVelocity;
    Renderer::PingPong mDensity;
    Renderer::PingPong mPressure;
    Renderer::RenderTexture mBoundaries;
    Renderer::RenderTexture mBoundariesVelocity;
    Renderer::RenderTexture mWeights;

    SuccessiveOverRelaxation mLinearSolver;
    Renderer::Reader mReader;

    Renderer::Program mAdvectShader;
    Renderer::Program mDivShader;
    Renderer::Program mProjectShader;
    Renderer::Program mAdvectDensityShader;
    Renderer::Program * mBoundarieshader;
    Renderer::Program * mIdentityShader;
    Renderer::Program mWeightsShader;

    float mDt;

    std::vector<FluidInput*> mFluidInputs;

    Renderer::Sprite mDensitySprite;

    Renderer::Rectangle mHorizontal;
    Renderer::Rectangle mVertical;
};

}

#endif /* defined(__Vortex__Engine__) */
