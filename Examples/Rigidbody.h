//
//  Rigidbody.h
//  Vortex2D
//

#ifndef Examples_Rigidbody_h
#define Examples_Rigidbody_h

#include <glm/trigonometric.hpp>

#include <Vortex2D/Engine/World.h>
#include <Vortex2D/Engine/Boundaries.h>
#include <Vortex2D/Engine/Rigidbody.h>
#include <Vortex2D/Engine/Size.h>
#include <Box2D/Box2D.h>

const float box2dScale = 32.0f;

class PolygonRigidbody
{
public:
  PolygonRigidbody(const Vortex2D::Renderer::Device& device,
                   b2World& rWorld,
                   const Vortex2D::Fluid::Dimensions& dimensions,
                   Vortex2D::Fluid::World& world,
                   b2BodyType type,
                   const std::vector<glm::vec2>& points);

  b2Body& Body();
  Vortex2D::Renderer::Drawable& SignedObject();

  void UpdatePosition();
  void UpdateVelocities();
  void SetTransform(const glm::vec2& pos, float angle);

private:
  float mScale;
  Vortex2D::Fluid::Polygon mDrawPolygon;
  Vortex2D::Fluid::RigidbodyRef mRigidbody;
  b2Body* mB2Body;
};

class BoxRigidbody : public PolygonRigidbody
{
public:
  BoxRigidbody(const Vortex2D::Renderer::Device& device,
               b2World& rWorld,
               const Vortex2D::Fluid::Dimensions& dimensions,
               Vortex2D::Fluid::World& world,
               b2BodyType type,
               const glm::vec2& halfSize)
    : PolygonRigidbody(device, rWorld, dimensions, world, type,
                      {{-halfSize.x, -halfSize.y},
                      {halfSize.x, -halfSize.y},
                      {halfSize.x, halfSize.y},
                      {-halfSize.x, halfSize.y}})
  {
  }

};

#endif
