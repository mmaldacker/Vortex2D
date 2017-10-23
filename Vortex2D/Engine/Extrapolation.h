//
//  Extrapolation.h
//  Vortex2D
//

#ifndef Extrapolation_h
#define Extrapolation_h

#include <Vortex2D/Renderer/CommandBuffer.h>
#include <Vortex2D/Renderer/Work.h>
#include <Vortex2D/Engine/LevelSet.h>

namespace Vortex2D { namespace Fluid {

/**
 * @brief Class to extrapolate values into the neumann and/or dirichlet boundaries
 */
class Extrapolation
{
public:
    Extrapolation(const Renderer::Device& device,
                  const glm::ivec2& size,
                  Renderer::GenericBuffer& valid,
                  Renderer::Texture& velocity,
                  int iterations = 10);

    /**
     * @brief Will extrapolate values from buffer into the dirichlet and neumann boundaries
     * @param buffer needs to be double-buffered and with stencil buffer
     */
    void Extrapolate();

    void ConstrainInit(Renderer::Texture& solidVelocity, Renderer::Texture& solidPhi);
    void ConstrainVelocity();


private:
    Renderer::Buffer<glm::ivec2> mValid;
    Renderer::Texture& mVelocity;
    // TODO use a common temp velocity texture
    Renderer::Texture mTempVelocity;

    Renderer::Work mExtrapolateVelocity;
    Renderer::Work::Bound mExtrapolateVelocityFrontBound, mExtrapolateVelocityBackBound;
    Renderer::Work mConstrainVelocity;
    Renderer::Work::Bound mConstrainVelocityBound;

    Renderer::CommandBuffer mExtrapolateCmd;
    Renderer::CommandBuffer mConstrainCmd;
};

}}

#endif
