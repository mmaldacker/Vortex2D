//
//  Transformable.h
//  Vortex2D
//

#ifndef Vortex2d_Transformable_h
#define Vortex2d_Transformable_h

#include <Vortex2D/Renderer/Common.h>

namespace Vortex2D
{
namespace Renderer
{
/**
 * @brief Class to represent the transformation of an object: position, scale,
 * rotation and anchor.
 */
struct Transformable
{
  Transformable();

  virtual ~Transformable() = default;

  /**
   * @brief Returns the transform matrix
   */
  const glm::mat4& GetTransform() const;

  /**
   * @brief absolute position
   */
  glm::vec2 Position;

  /**
   * @brief scale for the x and y components
   */
  glm::vec2 Scale;

  /**
   * @brief Rotation in radians
   */
  float Rotation;

  /**
   * @brief An offset to the position (used for centering a shape)
   */
  glm::vec2 Anchor;

  /**
   * @brief Update the transormation matrix
   */
  void Update();

private:
  glm::mat4 mTransform;
};

}  // namespace Renderer
}  // namespace Vortex2D

#endif
