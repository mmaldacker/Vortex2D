//
//  RenderState.h
//  Vortex2D
//

#ifndef Vortex_RenderState_h
#define Vortex_RenderState_h

#include <Vortex2D/Renderer/Common.h>

namespace Vortex2D { namespace Renderer {

class RenderState
{
public:
    int Width;
    int Height;
    vk::RenderPass RenderPass;
};

}}

#endif
