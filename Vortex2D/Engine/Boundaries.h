//
//  Boundaries.h
//  Vortex
//

#ifndef Vortex_Boundaries_h
#define Vortex_Boundaries_h

#include "LevelSet.h"
#include "Size.h"
#include "Drawable.h"

namespace Vortex2D { namespace Fluid {


class Boundaries
{
public:
    Boundaries(Dimensions dimensions, LevelSet& liquidPhi, LevelSet& solidPhi);
    ~Boundaries();

    void DrawLiquid(Renderer::Drawable& drawable, bool invert = false);
    void DrawSolid(Renderer::Drawable& drawable, bool invert = false);

    void ClearSolid();
    void ClearLiquid();

private:
    Dimensions mDimensions;
    LevelSet& mLiquidPhi;
    LevelSet& mSolidPhi;
    Renderer::RenderTexture mLiquidDraw;
    Renderer::RenderTexture mSolidDraw;
    Renderer::Operator mSolidBoundaries;
    Renderer::Operator mLiquidBoundaries;
};

}}

#endif
