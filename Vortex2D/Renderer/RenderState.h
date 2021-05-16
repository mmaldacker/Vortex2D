//
//  RenderState.h
//  Vortex2D
//

#ifndef Vortex2d_RenderState_h
#define Vortex2d_RenderState_h

#include <Vortex2D/Renderer/Common.h>

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
  VORTEX2D_API ColorBlendState();

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
   * @param blendState
   */
  RenderState(const RenderTarget& renderTarget, ColorBlendState blendState);

  uint32_t Width;
  uint32_t Height;
  vk::RenderPass RenderPass;
  ColorBlendState BlendState;
};

bool operator==(const RenderState& left, const RenderState right);

}  // namespace Renderer
}  // namespace Vortex2D

#endif
