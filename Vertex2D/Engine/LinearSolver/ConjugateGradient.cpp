//
//  ConjugateGradient.cpp
//  Vertex2D
//
//  Created by Maximilian Maldacker on 15/09/2015.
//  Copyright (c) 2015 Maximilian Maldacker. All rights reserved.
//

#include "ConjugateGradient.h"

namespace Fluid
{

void ConjugateGradient::Init(Boundaries & boundaries)
{

}

LinearSolver::Data & ConjugateGradient::GetData()
{
    return mMultigrid.GetData();
}

void ConjugateGradient::Solve()
{
    // initialise
    // x is solution

    // r = b - Ax
    // p is result from multigrid from r
    // rho = pTr

    // iterate

    // z = Ap
    // alpha = rho / pTz
    // r = r - alpha z
    // z is result from multigrid from r
    // rho_new = zTr
    // beta = rho_new / rho
    // rho = rho_new
    // x = x + alpha p
    // p = z + beta p

}

}