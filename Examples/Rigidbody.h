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
                   const std::vector<glm::vec2>& points)
    : mDrawPolygon(device, points)
  {
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

  b2Body& Body()
  {
    return *mB2Body;
  }

  Vortex2D::Fluid::SignedObject& SignedObject()
  {
    return mDrawPolygon;
  }

  void Update()
  {
    auto pos = mB2Body->GetPosition();
    mDrawPolygon.Position = {pos.x * box2dScale, pos.y * box2dScale};
    mDrawPolygon.Rotation = glm::degrees(mB2Body->GetAngle());
  }

  void SetTransform(const glm::vec2& pos, float angle)
  {
    mDrawPolygon.Position = pos;
    mDrawPolygon.Rotation = angle;
    mB2Body->SetTransform({pos.x / box2dScale, pos.y / box2dScale}, angle);
  }

private:
  Vortex2D::Fluid::Polygon mDrawPolygon;
  b2Body* mB2Body;

};

class BoxRigidbody : public PolygonRigidbody
{
public:
  BoxRigidbody(const Vortex2D::Renderer::Device& device, b2World& world, const glm::vec2& halfSize)
    : PolygonRigidbody(device, world,
  {{-halfSize.x, -halfSize.y},
  {halfSize.x, -halfSize.y},
  {halfSize.x, halfSize.y},
  {-halfSize.x, halfSize.y}})
  {
  }

};
