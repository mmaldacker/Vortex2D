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

class Rigidbody
{
public:
    Rigidbody(Vortex2D::Fluid::Dimensions dimensions,
              b2World& rWorld,
              b2BodyType rType,
              b2FixtureDef fixtureDef,
              float density = 1.0f);

    b2Body& Body();
    Vortex2D::Renderer::RenderTexture& Phi();

    void Update();
    void SetTransform(const glm::vec2& pos, float angle);

    void CreateBody(Vortex2D::Fluid::World& world,
                    Vortex2D::Fluid::RigidBody::Type type,
                    Vortex2D::Renderer::Drawable& drawable);
private:
    float mScale;
    Vortex2D::Fluid::RigidBody* mRigidbody;
    b2Body* mB2Body;
};

class PolygonRigidbody : public Rigidbody
{
public:
    PolygonRigidbody(const Vortex2D::Renderer::Device& device,
                     Vortex2D::Fluid::Dimensions dimensions,
                     b2World& rWorld,
                     b2BodyType rType,
                     Vortex2D::Fluid::World& world,
                     Vortex2D::Fluid::RigidBody::Type type,
                     const std::vector<glm::vec2>& points,
                     float density = 1.0f);

private:
    Vortex2D::Fluid::Polygon mPolygon;
};

class BoxRigidbody : public PolygonRigidbody
{
public:
    BoxRigidbody(const Vortex2D::Renderer::Device& device,
                 Vortex2D::Fluid::Dimensions dimensions,
                 b2World& rWorld,
                 b2BodyType rType,
                 Vortex2D::Fluid::World& world,
                 Vortex2D::Fluid::RigidBody::Type type,
                 const glm::vec2& halfSize,
                 float density = 1.0f)
        : PolygonRigidbody(device, dimensions, rWorld, rType, world, type,
                           {{-halfSize.x, -halfSize.y},
                           {halfSize.x, -halfSize.y},
                           {halfSize.x, halfSize.y},
                           {-halfSize.x, halfSize.y}},
                           density)
    {
    }
};

class CircleRigidbody : public Rigidbody
{
public:
    CircleRigidbody(const Vortex2D::Renderer::Device& device,
                    Vortex2D::Fluid::Dimensions dimensions,
                    b2World& rWorld,
                    b2BodyType rType,
                    Vortex2D::Fluid::World& world,
                    Vortex2D::Fluid::RigidBody::Type type,
                    const float radius,
                    float density = 1.0f);

private:
    Vortex2D::Fluid::Circle mCircle;
};

#endif
