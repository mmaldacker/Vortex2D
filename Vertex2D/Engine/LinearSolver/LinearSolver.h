//
//  LinearSolver.h
//  Vertex2D
//
//  Created by Maximilian Maldacker on 19/08/2015.
//  Copyright (c) 2015 Maximilian Maldacker. All rights reserved.
//

#ifndef Vertex2D_LinearSolver_h
#define Vertex2D_LinearSolver_h

#include "Boundaries.h"
#include "Shader.h"
#include "RenderTexture.h"

namespace Fluid
{

class LinearSolver
{
public:
    virtual void Init(Boundaries & boundaries) = 0;
    virtual void Render(Renderer::Program & program) = 0;
    virtual void BindWeights(int n) = 0;
    virtual Renderer::PingPong & GetPressure() = 0;
    virtual void Solve() = 0;
};

}

#endif
