//
//  Transformable.cpp
//  Vortex2D
//

#include "Transformable.h"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/transform.hpp>

namespace Vortex
{
namespace Renderer
{
Transformable::Transformable()
    : Position(0.0f, 0.0f), Scale(1.0f, 1.0f), Rotation(0.0f), Anchor(0.0f, 0.0f)
{
}

void Transformable::Update()
{
  mTransform = glm::translate(glm::vec3{Position, 0.0f});
  mTransform = glm::scale(mTransform, glm::vec3{Scale, 1.0f});
  mTransform = glm::rotate(mTransform, glm::radians(Rotation), glm::vec3{0.0f, 0.0f, 1.0f});
  mTransform = glm::translate(mTransform, glm::vec3{-Anchor, 0.0f});
}

const glm::mat4& Transformable::GetTransform() const
{
  return mTransform;
}

}  // namespace Renderer
}  // namespace Vortex2D
