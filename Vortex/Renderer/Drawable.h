//
//  Drawable.h
//  Vortex
//

#pragma once

#include <Vortex/Renderer/Common.h>

namespace Vortex
{
namespace Renderer
{
struct RenderState;
class CommandEncoder;

/**
 * @brief Interface of a drawable object.
 */
struct Drawable
{
  virtual ~Drawable() = default;

  /**
   * @brief Initialize the drawable for a particular state. This might include
   * creating the correct pipeline. If it was already initialized, it will do
   * nothing.
   * @param renderState the state to initialize with.
   */
  virtual void Initialize(const RenderState& renderState) = 0;

  /**
   * @brief Update the MVP matrix of the drawable
   * @param projection the projection matrix
   * @param view the view matrix
   */
  virtual void Update(const glm::mat4& projection, const glm::mat4& view) = 0;

  /**
   * @brief Draw for the given render state. This has to be initialized before.
   * @param commandBuffer the command buffer to record into.
   * @param renderState the render state to use.
   */
  virtual void Draw(CommandEncoder& commandEncoder, const RenderState& renderState) = 0;
};

using DrawablePtr = std::shared_ptr<Drawable>;

}  // namespace Renderer
}  // namespace Vortex
