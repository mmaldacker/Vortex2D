//
//  Multigrid.h
//  Vertex2D
//
//  Created by Maximilian Maldacker on 10/09/2015.
//  Copyright (c) 2015 Maximilian Maldacker. All rights reserved.
//

#ifndef __Vertex2D__Multigrid__
#define __Vertex2D__Multigrid__

#include "Size.h"
#include "RenderTexture.h"
#include "Boundaries.h"
#include "LinearSolver.h"

namespace Fluid
{

class Multigrid : public LinearSolver
{
public:
    Multigrid(Dimensions dimensions);

    void Init(Boundaries & boundaries) override;
    void Render(Renderer::Program & program) override;
    void BindWeights(int n) override;
    void BindPressure(int n) override;
    void Solve() override;

    Renderer::Reader GetPressureReader(int depth);

//private:
    void DampedJacobi(int depth);
    void Residual(int depth);
    void Restrict(int depth);
    void Prolongate(int depth);
    void Correct(int depth);

    std::vector<Renderer::Quad> mQuads;
    std::vector<Renderer::PingPong> mXs;
    std::vector<Renderer::RenderTexture> mWeightss;

    Renderer::Program mCorrectShader;
    Renderer::Program mDampedJacobiShader;
    Renderer::Program mProlongateShader;
    Renderer::Program mResidualShader;
    Renderer::Program mRestrictShader;
};

}

#endif /* defined(__Vertex2D__Multigrid__) */
