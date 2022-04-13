//
//  RenderState.h
//  Vortex
//

#pragma once

#include <Vortex/Renderer/Common.h>

#include <array>

namespace Vortex
{
namespace Renderer
{
struct RenderTarget;

/**
 * @brief The blend state and blend constant.
 */
struct ColorBlendState
{
  VORTEX_API ColorBlendState();

  VORTEX_API ColorBlendState(BlendFactor src, BlendFactor dst, BlendOp colorBlend);

  VORTEX_API ColorBlendState(BlendFactor src,
                             BlendFactor dst,
                             BlendOp colorBlend,
                             BlendFactor srcAlpha,
                             BlendFactor dstAlpha,
                             BlendOp alphaBlend);

  bool Enabled;

  BlendFactor Src;
  BlendFactor Dst;
  BlendOp ColorBlend;

  BlendFactor SrcAlpha;
  BlendFactor DstAlpha;
  BlendOp AlphaBlend;

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
   * @param blendState
   */
  RenderState(const RenderTarget& renderTarget, ColorBlendState blendState);

  uint32_t Width;
  uint32_t Height;
  Handle::RenderPass RenderPass;
  ColorBlendState BlendState;
};

bool operator==(const RenderState& left, const RenderState right);

}  // namespace Renderer
}  // namespace Vortex
