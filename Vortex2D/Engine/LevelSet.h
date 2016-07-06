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

// FIXME documentation
class LevelSet : public Buffer
{
public:
    LevelSet(const glm::vec2 & size);

    using Buffer::operator=;

    void Redistance(bool reinitialise = false);
    void RenderMask(Buffer & buffer);

private:
    Buffer mLevelSet0;
    Operator mRedistance;
    Operator mIdentity;
    Operator mMask;
};

}}

#endif /* LevelSet_h */
