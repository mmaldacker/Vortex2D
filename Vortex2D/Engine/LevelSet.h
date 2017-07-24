//
//  LevelSet.h
//  Vortex2D
//

#ifndef LevelSet_h
#define LevelSet_h

#include <Vortex2D/Renderer/RenderTexture.h>
#include <Vortex2D/Renderer/Work.h>
#include <Vortex2D/Renderer/CommandBuffer.h>

namespace Vortex2D { namespace Fluid {

/**
 * @brief A LevelSet class, which extends the Buffer class, with one dimension
 * The additional functionality ensures that the buffer represents a distance set.
 * This set can be reinitialised (or improved).
 */
class LevelSet : public Renderer::RenderTexture
{
public:
    LevelSet(const Renderer::Device& device, const glm::vec2& size);

    /**
     * @brief Iterative improvements to transform buffer into distance function
     * @param iterations number of iterations (1-3 after advection, 100 for reinitialisation)
     */
    void Redistance(int iterations);

    void Extrapolate(Renderer::Buffer& solidPhi);

private:
    Renderer::Texture mLevelSet0;
    Renderer::Texture mLevelSetBack;

    Renderer::Work mExtrapolate;
    Renderer::Work::Bound mExtrapolateBound;
    Renderer::Work mRedistance;
    Renderer::Work::Bound mRedistanceFront;
    Renderer::Work::Bound mRedistanceBack;

    Renderer::CommandBuffer mExtrapolateCmd;
    Renderer::CommandBuffer mReinitialiseCmd;
    Renderer::CommandBuffer mRedistanceCmd;
};

}}

#endif
