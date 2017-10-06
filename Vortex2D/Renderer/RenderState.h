//
//  RenderState.h
//  Vortex2D
//

#ifndef Vortex_RenderState_h
#define Vortex_RenderState_h

#include <Vortex2D/Renderer/Common.h>

namespace Vortex2D { namespace Renderer {

struct RenderTarget;

class RenderState
{
public:
    RenderState(const RenderTarget& renderTarget);
    RenderState(const RenderTarget& renderTarget,
                vk::PipelineColorBlendAttachmentState colorBlend);

    uint32_t Width;
    uint32_t Height;
    vk::RenderPass RenderPass;
    vk::PipelineColorBlendAttachmentState ColorBlend;
};

bool operator==(const RenderState& left, const RenderState right);

}}

#endif
