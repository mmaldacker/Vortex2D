//
//  RenderState.cpp
//  Vortex2D
//

#include "RenderState.h"

#include <Vortex2D/Renderer/RenderTarget.h>

namespace Vortex2D { namespace Renderer {

RenderState::RenderState(const RenderTarget& renderTarget)
    : Width(renderTarget.Width)
    , Height(renderTarget.Height)
    , RenderPass(*renderTarget.RenderPass)
{

}

RenderState::RenderState(const RenderTarget& renderTarget,
                         vk::PipelineColorBlendAttachmentState colorBlend)
    : RenderState(renderTarget)
{
    ColorBlend = colorBlend
        .setColorWriteMask(vk::ColorComponentFlagBits::eR |
                           vk::ColorComponentFlagBits::eG |
                           vk::ColorComponentFlagBits::eB |
                           vk::ColorComponentFlagBits::eA);
}

bool operator==(const RenderState& left, const RenderState right)
{
    return left.RenderPass == right.RenderPass &&
           left.ColorBlend == right.ColorBlend;
}

}}
