//
//  RenderTexture.h
//  Vortex
//

#pragma once

#include <Vortex/Renderer/CommandBuffer.h>
#include <Vortex/Renderer/Common.h>
#include <Vortex/Renderer/Device.h>
#include <Vortex/Renderer/Pipeline.h>
#include <Vortex/Renderer/RenderTarget.h>
#include <Vortex/Renderer/Texture.h>

namespace Vortex
{
namespace Renderer
{
class RenderCommand;

/**
 * @brief A render target that renders into a texture.
 */
class RenderTexture : public RenderTarget, public Texture
{
public:
  VORTEX_API RenderTexture(Device& device,
                           uint32_t width,
                           uint32_t height,
                           vk::Format format);

  VORTEX_API RenderTexture(RenderTexture&& other);

  VORTEX_API ~RenderTexture() override;

  VORTEX_API RenderCommand Record(DrawableList drawables, ColorBlendState blendState = {}) override;
  VORTEX_API void Submit(RenderCommand& renderCommand) override;

private:
  Device& mDevice;
  vk::UniqueFramebuffer mFramebuffer;
};

}  // namespace Renderer
}  // namespace Vortex
