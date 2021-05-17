//
//  Density.h
//  Vortex
//

#pragma once

#include <Vortex/Renderer/RenderTexture.h>
#include <Vortex/Renderer/Sprite.h>

namespace Vortex
{
namespace Fluid
{
/**
 * @brief Density field, used to represent smoke swirling.
 */
class Density : public Renderer::RenderTexture, public Renderer::Sprite
{
public:
  VORTEX_API Density(const Renderer::Device& device, const glm::ivec2& size, vk::Format format);

  friend class Advection;

private:
  Renderer::Texture mFieldBack;
};

}  // namespace Fluid
}  // namespace Vortex
