//
//  Rigidbody.cpp
//  Vortex
//

#include "Rigidbody.h"

#include <iostream>

namespace
{
float sign(float value)
{
  if (value >= 0.0f)
    return 1.0f;
  return -1.0;
}

b2FixtureDef GetPolygonFixtureDef(const std::vector<glm::vec2>& points)
{
  static b2PolygonShape shape;

  std::vector<b2Vec2> b2Points;
  for (auto& point : points)
  {
    b2Points.push_back(
        {point.x - sign(point.x) * b2_polygonRadius, point.y - sign(point.y) * b2_polygonRadius});
  }

  shape.Set(b2Points.data(), static_cast<int>(b2Points.size()));

  static b2FixtureDef fixtureDef;
  fixtureDef.shape = &shape;

  return fixtureDef;
}

b2FixtureDef GetCircleFixtureDef(float radius)
{
  static b2CircleShape shape;
  shape.m_radius = radius;

  static b2FixtureDef fixtureDef;
  fixtureDef.shape = &shape;

  return fixtureDef;
}

b2Body* CreateBody(b2World& world, b2FixtureDef fixtureDef, b2BodyType type, float density)
{
  b2BodyDef def;
  def.type = type;

  auto* body = world.CreateBody(&def);

  fixtureDef.density = density;
  body->CreateFixture(&fixtureDef);

  return body;
}

}  // namespace

Box2DSolver::Box2DSolver(b2World& world) : mWorld(world) {}

void Box2DSolver::Step(float delta)
{
  const int velocityStep = 8;
  const int positionStep = 3;
  mWorld.Step(delta, velocityStep, positionStep);
}

Box2DRigidbody::Box2DRigidbody(Vortex::Renderer::Device& device,
                               const glm::ivec2& size,
                               Vortex::Renderer::DrawablePtr drawable,
                               Vortex::Fluid::RigidBody::Type type)
    : Vortex::Fluid::RigidBody(device, size, drawable, type)
{
}

void Box2DRigidbody::ApplyForces()
{
  if (GetType() & Vortex::Fluid::RigidBody::Type::eWeak)
  {
    auto force = GetForces();
    b2Vec2 b2Force = {force.velocity.x, force.velocity.y};

    mBody->ApplyForceToCenter(b2Force, true);
    mBody->ApplyTorque(force.angular_velocity, true);
  }
}

void Box2DRigidbody::ApplyVelocities()
{
  auto pos = mBody->GetPosition();
  Position = {pos.x, pos.y};
  Rotation = glm::degrees(mBody->GetAngle());

  if (GetType() & Vortex::Fluid::RigidBody::Type::eStatic)
  {
    glm::vec2 vel = {mBody->GetLinearVelocity().x, mBody->GetLinearVelocity().y};
    float angularVelocity = mBody->GetAngularVelocity();
    SetVelocities(vel, angularVelocity);
  }
}

void Box2DRigidbody::SetTransform(const glm::vec2& pos, float angle)
{
  Position = pos;
  Rotation = angle;
  mBody->SetTransform({pos.x, pos.y}, glm::radians(angle));
}

PolygonRigidbody::PolygonRigidbody(Vortex::Renderer::Device& device,
                                   const glm::ivec2& size,
                                   b2World& rWorld,
                                   b2BodyType rType,
                                   Vortex::Fluid::RigidBody::Type type,
                                   const std::vector<glm::vec2>& points,
                                   float density)
    : mPolygon(std::make_shared<Vortex::Fluid::Polygon>(device, points))
    , mRigidbody(device, size, mPolygon, type)
{
  mRigidbody.mBody = CreateBody(rWorld, GetPolygonFixtureDef(points), rType, density);
  mRigidbody.SetMassData(mRigidbody.mBody->GetMass(), mRigidbody.mBody->GetInertia());
}

CircleRigidbody::CircleRigidbody(Vortex::Renderer::Device& device,
                                 const glm::ivec2& size,
                                 b2World& rWorld,
                                 b2BodyType rType,
                                 Vortex::Fluid::RigidBody::Type type,
                                 const float radius,
                                 float density)
    : mCircle(std::make_shared<Vortex::Fluid::Circle>(device, radius))
    , mRigidbody(device, size, mCircle, type)
{
  mRigidbody.mBody = CreateBody(rWorld, GetCircleFixtureDef(radius), rType, density);
  mRigidbody.SetMassData(mRigidbody.mBody->GetMass(), mRigidbody.mBody->GetInertia());
}
