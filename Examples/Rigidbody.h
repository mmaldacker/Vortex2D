//
//  Rigidbody.h
//  Vortex2D
//

#include <glm/trigonometric.hpp>

#include <Vortex2D/Engine/Boundaries.h>
#include <Box2D/Box2D.h>

const float box2dScale = 32.0f;

class PolygonRigidbody
{
public:
  PolygonRigidbody(const Vortex2D::Renderer::Device& device,
                   b2World& world,
                   const glm::ivec2& size,
                   const std::vector<glm::vec2>& points);

  b2Body& Body();

  Vortex2D::Fluid::SignedObject& SignedObject();
  Vortex2D::Renderer::Drawable& VelocityObject();

  void Update();
  void UpdateVelocities();
  void Update(const glm::mat4& projection, const glm::mat4& view);
  void SetTransform(const glm::vec2& pos, float angle);

private:
  float mScale;
  Vortex2D::Renderer::AbstractShape mDrawPolygon;
  Vortex2D::Fluid::PolygonVelocity mVelocityPolygon;
  b2Body* mB2Body;
};

class BoxRigidbody : public PolygonRigidbody
{
public:
  BoxRigidbody(const Vortex2D::Renderer::Device& device,
               b2World& world,
               const glm::ivec2& size,
               const glm::vec2& halfSize)
    : PolygonRigidbody(device, world, size,
                      {{-halfSize.x, -halfSize.y},
                      {halfSize.x, -halfSize.y},
                      {halfSize.x, halfSize.y},
                      {-halfSize.x, halfSize.y}})
  {
  }

};
