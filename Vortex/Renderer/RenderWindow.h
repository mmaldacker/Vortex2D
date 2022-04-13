//
//  RenderWindow.h
//  Vortex
//

#pragma once

#include <Vortex/Renderer/Common.h>
#include <Vortex/Renderer/Device.h>
#include <Vortex/Renderer/Pipeline.h>
#include <Vortex/Renderer/RenderTarget.h>

namespace Vortex
{
namespace Renderer
{
/**
 * @brief Render to a swapchain, i.e. to the window/surface.
 */
class RenderWindow : public RenderTarget
{
public:
  /**
   * @brief Initialize with a given surface and size.
   * @param device vulkan device
   * @param surface vulkan surface
   * @param width
   * @param height
   */
  VORTEX_API RenderWindow(Device& device, Handle::Surface surface, uint32_t width, uint32_t height);
  VORTEX_API ~RenderWindow() override;

  VORTEX_API RenderCommand Record(DrawableList drawables, ColorBlendState blendState = {}) override;
  VORTEX_API void Submit(RenderCommand& renderCommand) override;

  /**
   * @brief Submits all the render command and present the surface for display.
   */
  VORTEX_API void Display();

private:
  struct Impl;
  std::unique_ptr<Impl> mImpl;
};

}  // namespace Renderer
}  // namespace Vortex
