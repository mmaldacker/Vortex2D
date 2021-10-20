//
//  Rigidbody.h
//  Vortex
//

#pragma once

#include <glm/trigonometric.hpp>

#include <Box2D/Box2D.h>
#include <Vortex/Engine/Boundaries.h>
#include <Vortex/Engine/Rigidbody.h>
#include <Vortex/Engine/World.h>

class Box2DSolver : public Vortex::Fluid::RigidBodySolver
{
public:
  Box2DSolver(b2World& world);
  void Step(float delta) override;

private:
  b2World& mWorld;
};

class Box2DRigidbody : public Vortex::Fluid::RigidBody
{
public:
  Box2DRigidbody(Vortex::Renderer::Device& device,
                 const glm::ivec2& size,
                 Vortex::Renderer::DrawablePtr drawable,
                 Vortex::Fluid::RigidBody::Type type);

  void ApplyForces();
  void ApplyVelocities();

  void SetTransform(const glm::vec2& pos, float angle);

  b2Body* mBody;
};

class PolygonRigidbody
{
public:
  PolygonRigidbody(Vortex::Renderer::Device& device,
                   const glm::ivec2& size,
                   b2World& rWorld,
                   b2BodyType rType,
                   Vortex::Fluid::RigidBody::Type type,
                   const std::vector<glm::vec2>& points,
                   float density = 1.0f);

  std::shared_ptr<Vortex::Fluid::Polygon> mPolygon;
  Box2DRigidbody mRigidbody;
};

class RectangleRigidbody : public PolygonRigidbody
{
public:
  RectangleRigidbody(Vortex::Renderer::Device& device,
                     const glm::ivec2& size,
                     b2World& rWorld,
                     b2BodyType rType,
                     Vortex::Fluid::RigidBody::Type type,
                     const glm::vec2& halfSize,
                     float density = 1.0f)
      : PolygonRigidbody(device,
                         size,
                         rWorld,
                         rType,
                         type,
                         {{-halfSize.x, -halfSize.y},
                          {halfSize.x, -halfSize.y},
                          {halfSize.x, halfSize.y},
                          {-halfSize.x, halfSize.y}},
                         density)
  {
  }
};

class CircleRigidbody
{
public:
  CircleRigidbody(Vortex::Renderer::Device& device,
                  const glm::ivec2& size,
                  b2World& rWorld,
                  b2BodyType rType,
                  Vortex::Fluid::RigidBody::Type type,
                  const float radius,
                  float density = 1.0f);

  std::shared_ptr<Vortex::Fluid::Circle> mCircle;
  Box2DRigidbody mRigidbody;
};
