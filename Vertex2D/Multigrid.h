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
#include "SuccessiveOverRelaxation.h"

namespace Fluid
{

class Multigrid : public LinearSolver
{
public:
    Multigrid(const glm::vec2 & size, int iterations = 4);

    void Init(Boundaries & boundaries) override;
    void Render(Renderer::Program & program) override;
    void BindWeights(int n) override;
    Renderer::PingPong & GetPressure() override;
    void Solve() override;

    Renderer::Reader GetPressureReader(int depth);

//private:
    void DampedJacobi(int depth);
    void Residual(int depth);
    void Restrict(int depth);
    void Prolongate(int depth);
    void Correct(int depth);

    int mDepths;

    std::vector<SuccessiveOverRelaxation> mXs;

    Renderer::Program mCorrectShader;
    Renderer::Program mProlongateShader;
    Renderer::Program mResidualShader;
    Renderer::Program mRestrictShader;
};

}

#endif /* defined(__Vertex2D__Multigrid__) */
