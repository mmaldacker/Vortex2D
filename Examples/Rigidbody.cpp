//
//  Rigidbody.cpp
//  Vortex2D
//

#include "Rigidbody.h"


PolygonRigidbody::PolygonRigidbody(const Vortex2D::Renderer::Device& device,
                                   b2World& world,
                                   const glm::ivec2& size,
                                   const std::vector<glm::vec2>& points)
    : mDrawPolygon(device, points)
    , mVelocityPolygon(device, size, points, {})
{
  mVelocityPolygon.Scale = glm::vec2(2.0f);
  b2PolygonShape shape;

  std::vector<b2Vec2> b2Points;
  for (auto& point: points)
  {
    b2Points.push_back({point.x / box2dScale, point.y / box2dScale});
  }

  shape.Set(b2Points.data(), b2Points.size());

  b2FixtureDef fixtureDef;
  fixtureDef.shape = &shape;
  fixtureDef.density = 1.0f;

  b2BodyDef def;
  def.type = b2_dynamicBody;

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

void PolygonRigidbody::Update()
{
  auto pos = mB2Body->GetPosition();
  mVelocityPolygon.Position = mDrawPolygon.Position = {pos.x * box2dScale, pos.y * box2dScale};
  mVelocityPolygon.Rotation = mDrawPolygon.Rotation = glm::degrees(mB2Body->GetAngle());
}

void PolygonRigidbody::UpdateVelocities()
{
    glm::vec2 vel = {mB2Body->GetLinearVelocity().x, mB2Body->GetLinearVelocity().y};
    float angularVelocity = mB2Body->GetAngularVelocity();
    float scale = box2dScale / 4.0f;
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
