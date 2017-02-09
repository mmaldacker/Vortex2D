//
//  Extrapolation.h
//  Vortex2D
//

#ifndef Extrapolation_h
#define Extrapolation_h

#include <Vortex2D/Renderer/Operator.h>
#include <Vortex2D/Renderer/Buffer.h>
#include <Vortex2D/Renderer/Shapes.h>

#include <Vortex2D/Engine/Size.h>
#include <Vortex2D/Engine/LevelSet.h>

namespace Vortex2D { namespace Fluid {

/**
 * @brief Class to extrapolate values into the neumann and/or dirichlet boundaries
 */
class Extrapolation
{
public:
    Extrapolation(const glm::vec2& size,
                  Renderer::Buffer& velocity,
                  Renderer::Buffer& solidPhi);

    /**
     * @brief Will extrapolate values from buffer into the dirichlet and neumann boundaries
     * @param buffer needs to be double-buffered and with stencil buffer
     */
    void Extrapolate();

    void ConstrainVelocity();


private:
    Renderer::Buffer& mVelocity;
    Renderer::Buffer& mSolidPhi;

    Renderer::Buffer mExtrapolateValid;

    Renderer::Operator mIdentity;
    Renderer::Operator mExtrapolate;
    Renderer::Operator mValidExtrapolate;
    Renderer::Operator mValidVelocities;
    Renderer::Operator mConstrainVelocity;

    Renderer::Rectangle mSurface;
};

}}

#endif /* Extrapolation_h */
