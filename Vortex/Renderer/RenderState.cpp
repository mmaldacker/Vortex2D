//
//  RenderState.cpp
//  Vortex
//

#include "RenderState.h"

#include <Vortex/Renderer/RenderTarget.h>

namespace Vortex
{
namespace Renderer
{
ColorBlendState::ColorBlendState()
    : Enabled(false)
    , Src(BlendFactor::Zero)
    , Dst(BlendFactor::Zero)
    , ColorBlend(BlendOp::Add)
    , SrcAlpha(BlendFactor::Zero)
    , DstAlpha(BlendFactor::Zero)
    , AlphaBlend(BlendOp::Add)
    , BlendConstants({1.0f})
{
}

ColorBlendState::ColorBlendState(BlendFactor src, BlendFactor dst, BlendOp colorBlend)
    : Enabled(true)
    , Src(src)
    , Dst(dst)
    , ColorBlend(colorBlend)
    , SrcAlpha(BlendFactor::Zero)
    , DstAlpha(BlendFactor::Zero)
    , AlphaBlend(BlendOp::Add)
    , BlendConstants({1.0f})
{
}

ColorBlendState::ColorBlendState(BlendFactor src,
                                 BlendFactor dst,
                                 BlendOp colorBlend,
                                 BlendFactor srcAlpha,
                                 BlendFactor dstAlpha,
                                 BlendOp alphaBlend)
    : Enabled(true)
    , Src(src)
    , Dst(dst)
    , ColorBlend(colorBlend)
    , SrcAlpha(srcAlpha)
    , DstAlpha(dstAlpha)
    , AlphaBlend(alphaBlend)
    , BlendConstants({1.0f})
{
}

RenderState::RenderState(const RenderTarget& renderTarget, struct ColorBlendState blendState)
    : Width(renderTarget.GetWidth())
    , Height(renderTarget.GetHeight())
    , RenderPass(renderTarget.GetRenderPass())
    , BlendState(blendState)
{
}

RenderState::RenderState(const RenderTarget& renderTarget) : RenderState(renderTarget, {}) {}

bool operator==(const RenderState& left, const RenderState right)
{
  return left.Width == right.Width && left.Height == right.Height &&
         left.RenderPass == right.RenderPass &&
         left.BlendState.ColorBlend == right.BlendState.ColorBlend &&
         left.BlendState.BlendConstants == right.BlendState.BlendConstants;
}

}  // namespace Renderer
}  // namespace Vortex
