//
//  RenderState.h
//  Vortex2D
//

#ifndef Vortex2d_RenderState_h
#define Vortex2d_RenderState_h

#include <Vortex2D/Renderer/Common.h>

namespace Vortex2D { namespace Renderer {

struct RenderTarget;

/**
 * @brief The blend state and blend constant.
 */
struct BlendState
{
    VORTEX2D_API BlendState();

    vk::PipelineColorBlendAttachmentState ColorBlend;
    std::array<float, 4> BlendConstants;
};

/**
 * @brief the various state to render to a target: size, render pass and blend.
 */
struct RenderState
{
    /**
     * @brief Initialize for a render target with default blend
     * @param renderTarget
     */
    RenderState(const RenderTarget& renderTarget);

    /**
     * @brief Initialize for a render target with a given blend
     * @param renderTarget
     * @param colorBlend
     */
    RenderState(const RenderTarget& renderTarget,
                BlendState blendState);

    uint32_t Width;
    uint32_t Height;
    vk::RenderPass RenderPass;
    BlendState BlendState;
};

bool operator==(const RenderState& left, const RenderState right);

}}

#endif
