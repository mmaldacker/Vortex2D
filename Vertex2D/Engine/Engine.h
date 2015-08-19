//
//  GPUFluidEngine.h
//  Vortex
//
//  Created by Maximilian Maldacker on 08/04/2014.
//
//

#ifndef __Vortex__GPUFluidEngine__
#define __Vortex__GPUFluidEngine__

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

class GPUFluidEngine
{
public:
    GPUFluidEngine(const glm::vec2 & size, float scale, float dt, int antiAlias, int iterations);
    
    void Solve();
    void AddInput(FluidInput * input);
    Renderer::Sprite & GetDensity();
    void Reset();

    void Start();
    void Stop();

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

    Renderer::Quad mQuad;

    Renderer::PingPong mVelocity;
    Renderer::PingPong mDensity;
    Renderer::PingPong mPressure;
    Renderer::RenderTexture mObstacles;
    Renderer::RenderTexture mObstaclesVelocity;
    Renderer::RenderTexture mWeights;

    SuccessiveOverRelaxation mLinearSolver;
    Renderer::Reader mReader;

    Renderer::Program mAdvectShader;
    Renderer::Program mDivShader;
    Renderer::Program mProjectShader;
    Renderer::Program mAdvectDensityShader;
    Renderer::Program * mObstacleShader;
    Renderer::Program * mIdentityShader;
    Renderer::Program mWeightsShader;
    Renderer::Program mObstaclesVelocityShader;

    float mDt;

    std::vector<FluidInput*> mFluidInputs;

    Renderer::Sprite mDensitySprite;

    void PrintGrid(Renderer::RenderTexture & renderTexture, int format, int component);

    bool mRun;
    bool mDebug;

    Renderer::Rectangle mHorizontal;
    Renderer::Rectangle mVertical;
};

}

#endif /* defined(__Vortex__GPUFluidEngine__) */
