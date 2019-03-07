//
//  RenderTexture.h
//  Vortex2D
//

#ifndef RenderTexture_h
#define RenderTexture_h

#include <Vortex2D/Renderer/CommandBuffer.h>
#include <Vortex2D/Renderer/Common.h>
#include <Vortex2D/Renderer/Device.h>
#include <Vortex2D/Renderer/Pipeline.h>
#include <Vortex2D/Renderer/RenderTarget.h>
#include <Vortex2D/Renderer/Texture.h>

namespace Vortex2D
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
  VORTEX2D_API RenderTexture(const Device& device,
                             uint32_t width,
                             uint32_t height,
                             vk::Format format);

  VORTEX2D_API RenderTexture(RenderTexture&& other);

  VORTEX2D_API ~RenderTexture() override;

  VORTEX2D_API RenderCommand Record(DrawableList drawables,
                                    ColorBlendState blendState = {}) override;
  VORTEX2D_API void Submit(RenderCommand& renderCommand) override;

private:
  const Device& mDevice;
  vk::UniqueFramebuffer mFramebuffer;
};

}  // namespace Renderer
}  // namespace Vortex2D

#endif
