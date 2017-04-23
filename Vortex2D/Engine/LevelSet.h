//
//  LevelSet.h
//  Vortex2D
//

#ifndef LevelSet_h
#define LevelSet_h

#include <Vortex2D/Renderer/Data.h>
#include <Vortex2D/Renderer/Operator.h>
#include <Vortex2D/Renderer/Drawable.h>

namespace Vortex2D { namespace Fluid {

/**
 * @brief A LevelSet class, which extends the Buffer class, with one dimension
 * The additional functionality ensures that the buffer represents a distance set.
 * This set can be reinitialised (or improved).
 */
class LevelSet : public Renderer::Buffer
{
public:
    LevelSet(const glm::vec2& size);

    using Buffer::operator=;

    /**
     * @brief Iterative improvements to transform buffer into distance function
     * @param iterations number of iterations (1-3 after advection, 100 for reinitialisation)
     */
    void Redistance(int iterations);

    void Extrapolate(Renderer::Buffer& solidPhi);

private:
    Renderer::Buffer mLevelSet0;
    Renderer::Operator mRedistance;
    Renderer::Operator mIdentity;
    Renderer::Operator mExtrapolate;
};

}}

#endif
