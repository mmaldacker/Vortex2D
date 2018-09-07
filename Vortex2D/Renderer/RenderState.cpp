//
//  RenderState.cpp
//  Vortex2D
//

#include "RenderState.h"

#include <Vortex2D/Renderer/RenderTarget.h>

namespace Vortex2D { namespace Renderer {

ColorBlendState::ColorBlendState()
    : BlendConstants({1.0f})
{
    ColorBlend.setColorWriteMask(vk::ColorComponentFlagBits::eR |
                                 vk::ColorComponentFlagBits::eG |
                                 vk::ColorComponentFlagBits::eB |
                                 vk::ColorComponentFlagBits::eA);
}

RenderState::RenderState(const RenderTarget& renderTarget,
                         struct ColorBlendState blendState)
    : Width(renderTarget.Width)
    , Height(renderTarget.Height)
    , RenderPass(*renderTarget.RenderPass)
    , BlendState(blendState)
{

}

RenderState::RenderState(const RenderTarget& renderTarget)
    : RenderState(renderTarget, {})
{
}

bool operator==(const RenderState& left, const RenderState right)
{
    return left.RenderPass == right.RenderPass &&
           left.BlendState.ColorBlend == right.BlendState.ColorBlend &&
           left.BlendState.BlendConstants  == right.BlendState.BlendConstants;
}

}}
