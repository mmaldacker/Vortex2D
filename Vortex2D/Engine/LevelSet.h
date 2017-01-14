//
//  LevelSet.h
//  Vortex2D
//

#ifndef LevelSet_h
#define LevelSet_h

#include "Buffer.h"
#include "Operator.h"
#include "Drawable.h"

namespace Vortex2D { namespace Fluid {

/**
 * @brief A LevelSet class, which extends the Buffer class, with one dimension
 * The additional functionality ensures that the buffer represents a distance set.
 * This set can be reinitialised (or improved).
 */
class LevelSet : public Buffer
{
public:
    LevelSet(const glm::vec2 & size);

    using Buffer::operator=;

    /**
     * @brief Iterative improvements to transform buffer into distance function
     * @param iterations number of iterations (1-3 after advection, 100 for reinitialisation)
     */
    void Redistance(int iterations);

    void RenderMask(Buffer & buffer);

private:
    Buffer mLevelSet0;
    Operator mRedistance;
    Operator mIdentity;
    Operator mMask;
};

}}

#endif /* LevelSet_h */
