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
    Multigrid(Dimensions dimensions,
              Renderer::RenderTexture & weights,
              Renderer::PingPong & x);

    void Init(Boundaries & boundaries);
    void Solve() override;

    Renderer::Reader GetPressureReader(int depth);

//private:
    template<typename T>
    T & Get(int depth, T & s, std::vector<T> & l);

    Renderer::PingPong & GetX(int depth);
    Renderer::RenderTexture & GetWeights(int depth);
    Renderer::Quad & GetQuad(int depth);

    void DampedJacobi(int depth);
    void Residual(int depth);
    void Restrict(int depth);
    void Prolongate(int depth);
    void Correct(int depth);

    Renderer::PingPong & mX;
    Renderer::RenderTexture & mWeights;

    Renderer::Quad mQuad;
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
