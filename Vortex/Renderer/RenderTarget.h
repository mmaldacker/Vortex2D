//
//  RenderTarget.h
//  Vortex
//

#pragma once

#include <Vortex/Renderer/Common.h>
#include <Vortex/Renderer/RenderState.h>

#include <functional>
#include <initializer_list>

namespace Vortex
{
namespace Renderer
{
class RenderCommand;
class Device;
struct Drawable;

/**
 * @brief A target that can be rendered to.
 * This is implemented by the @ref RenderWindow and the @ref RenderTexture
 */
struct RenderTarget
{
  VORTEX_API RenderTarget(Device& device,
                          uint32_t width,
                          uint32_t height,
                          Handle::RenderPass renderPass);

  RenderTarget(RenderTarget&& other);

  VORTEX_API virtual ~RenderTarget();

  using DrawableList = std::vector<std::shared_ptr<Drawable>>;

  VORTEX_API virtual RenderCommand Record(DrawableList drawables,
                                          ColorBlendState blendState = {}) = 0;

  // TODO should use shared_ptr?
  VORTEX_API virtual void Submit(RenderCommand& renderCommand) = 0;

  uint32_t GetWidth() const;

  uint32_t GetHeight() const;

  const glm::mat4& GetOrth() const;

  const glm::mat4& GetView() const;

  VORTEX_API void SetView(const glm::mat4& view);

  Handle::RenderPass GetRenderPass() const;

private:
  struct Impl;
  std::unique_ptr<Impl> mImpl;
};

}  // namespace Renderer
}  // namespace Vortex
