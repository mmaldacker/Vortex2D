//
//  Rigidbody.cpp
//  Vortex2D
//

#include "Rigidbody.h"

namespace
{

float sign(float value)
{
    if (value >= 0.0f) return 1.0f;
    return -1.0;
}

}

PolygonRigidbody::PolygonRigidbody(const Vortex2D::Renderer::Device& device,
                                   b2World& world,
                                   const Vortex2D::Fluid::Dimensions& dimensions,
                                   Vortex2D::Renderer::GenericBuffer& valid,
                                   b2BodyType type,
                                   const std::vector<glm::vec2>& points)
    : mScale(dimensions.Scale)
    , mDrawPolygon(device, points)
    , mVelocityPolygon(device, dimensions.Size, valid, points, {})
{
  b2PolygonShape shape;

  std::vector<b2Vec2> b2Points;
  for (auto& point: points)
  {
    b2Points.push_back({point.x / box2dScale - sign(point.x) * b2_polygonRadius,
                        point.y / box2dScale - sign(point.y) * b2_polygonRadius});
  }

  shape.Set(b2Points.data(), b2Points.size());

  b2FixtureDef fixtureDef;
  fixtureDef.shape = &shape;
  fixtureDef.density = 1.0f;

  b2BodyDef def;
  def.type = type;

  mB2Body = world.CreateBody(&def);
  mB2Body->CreateFixture(&fixtureDef);
}

b2Body& PolygonRigidbody::Body()
{
  return *mB2Body;
}

Vortex2D::Fluid::SignedObject& PolygonRigidbody::SignedObject()
{
  return mDrawPolygon;
}

Vortex2D::Renderer::Drawable& PolygonRigidbody::VelocityObject()
{
    return mVelocityPolygon;
}

void PolygonRigidbody::UpdatePosition()
{
  auto pos = mB2Body->GetPosition();
  mVelocityPolygon.Position = mDrawPolygon.Position = {pos.x * box2dScale, pos.y * box2dScale};
  mVelocityPolygon.Rotation = mDrawPolygon.Rotation = glm::degrees(mB2Body->GetAngle());
}

void PolygonRigidbody::UpdateVelocities()
{
    glm::vec2 vel = {mB2Body->GetLinearVelocity().x, mB2Body->GetLinearVelocity().y};
    float angularVelocity = mB2Body->GetAngularVelocity();
    float scale = box2dScale / mScale;
    mVelocityPolygon.UpdateVelocities(vel * scale, angularVelocity);
}

void PolygonRigidbody::Update(const glm::mat4& projection, const glm::mat4& view)
{
    mDrawPolygon.Update(view);
    mVelocityPolygon.Update(projection, view);
}

void PolygonRigidbody::SetTransform(const glm::vec2& pos, float angle)
{
  mVelocityPolygon.Position = mDrawPolygon.Position = pos;
  mVelocityPolygon.Rotation = mDrawPolygon.Rotation = angle;
  mB2Body->SetTransform({pos.x / box2dScale, pos.y / box2dScale}, angle);
}
