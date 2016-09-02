//
//  Extrapolation.h
//  Vortex2D

#ifndef Extrapolation_h
#define Extrapolation_h

#include "Size.h"
#include "Operator.h"
#include "Buffer.h"
#include "Shapes.h"
#include "LevelSet.h"

namespace Vortex2D { namespace Fluid {

/**
 * @brief Class to extrapolate values into the neumann and/or dirichlet boundaries
 */
class Extrapolation
{
public:
    Extrapolation(Dimensions dimensions);

    /**
     * @brief Will extrapolate values from buffer into the dirichlet and neumann boundaries
     * @param buffer needs to be double-buffered and with stencil buffer
     */
    void Extrapolate(Buffer & buffer, LevelSet & neumann, LevelSet & dirichlet);

private:
    Buffer mExtrapolateValid;

    Operator mIdentity;
    Operator mExtrapolate;
    Operator mExtrapolateMask;

    Renderer::Rectangle mSurface;
};

}}

#endif /* Extrapolation_h */
