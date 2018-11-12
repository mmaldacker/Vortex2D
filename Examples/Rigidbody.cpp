//
//  Rigidbody.cpp
//  Vortex2D
//

#include "Rigidbody.h"

#include <iostream>

namespace
{

float sign(float value)
{
    if (value >= 0.0f) return 1.0f;
    return -1.0;
}

b2FixtureDef GetPolygonFixtureDef(const std::vector<glm::vec2>& points, float scale)
{
    static b2PolygonShape shape;

    std::vector<b2Vec2> b2Points;
    for (auto& point: points)
    {
        b2Points.push_back({point.x / scale - sign(point.x) * b2_polygonRadius,
                            point.y / scale - sign(point.y) * b2_polygonRadius});
    }

    shape.Set(b2Points.data(), static_cast<int>(b2Points.size()));

    static b2FixtureDef fixtureDef;
    fixtureDef.shape = &shape;

    return fixtureDef;
}

b2FixtureDef GetCircleFixtureDef(float radius, float scale)
{
    static b2CircleShape shape;
    shape.m_radius = radius / scale;

    static b2FixtureDef fixtureDef;
    fixtureDef.shape = &shape;

    return fixtureDef;
}

}

Rigidbody::Rigidbody(Vortex2D::Fluid::Dimensions dimensions,
                     b2World& rWorld,
                     b2BodyType rType,
                     b2FixtureDef fixtureDef,
                     float density)
    : mScale(dimensions.Scale)
{
    b2BodyDef def;
    def.type = rType;

    mB2Body = rWorld.CreateBody(&def);

    fixtureDef.density = density;
    mB2Body->CreateFixture(&fixtureDef);
}

void Rigidbody::CreateBody(Vortex2D::Fluid::World& world,
                           Vortex2D::Fluid::RigidBody::Type type,
                           Vortex2D::Renderer::Drawable& drawable)
{
    mRigidbody = world.CreateRigidbody(type,
                                       mB2Body->GetMass(),
                                       mB2Body->GetInertia(),
                                       drawable,
                                       {});
}

b2Body& Rigidbody::Body()
{
    return *mB2Body;
}

Vortex2D::Renderer::RenderTexture& Rigidbody::Phi()
{
    return mRigidbody->Phi();
}

void Rigidbody::Update()
{
    auto pos = mB2Body->GetPosition();
    mRigidbody->Position = {pos.x * mScale, pos.y * mScale};
    mRigidbody->Rotation = glm::degrees(mB2Body->GetAngle());
    mRigidbody->UpdatePosition();

    if (mRigidbody->GetType() & Vortex2D::Fluid::RigidBody::Type::eStatic)
    {
        glm::vec2 vel = {mB2Body->GetLinearVelocity().x * mScale, mB2Body->GetLinearVelocity().y * mScale};
        float angularVelocity = mB2Body->GetAngularVelocity();
        mRigidbody->SetVelocities(vel, angularVelocity);
    }

    if (mRigidbody->GetType() & Vortex2D::Fluid::RigidBody::Type::eWeak)
    {
        auto force = mRigidbody->GetForces();
        b2Vec2 b2Force = {force.velocity.x * mScale, force.velocity.y * mScale};

        mB2Body->ApplyForceToCenter(b2Force, true);
        mB2Body->ApplyTorque(force.angular_velocity, true);
    }
}

void Rigidbody::SetTransform(const glm::vec2& pos, float angle)
{
    mRigidbody->Position = pos;
    mRigidbody->Rotation = angle;
    mB2Body->SetTransform({pos.x / mScale, pos.y / mScale}, angle);
}

PolygonRigidbody::PolygonRigidbody(const Vortex2D::Renderer::Device& device,
                                   Vortex2D::Fluid::Dimensions dimensions,
                                   b2World& rWorld,
                                   b2BodyType rType,
                                   Vortex2D::Fluid::World& world,
                                   Vortex2D::Fluid::RigidBody::Type type,
                                   const std::vector<glm::vec2>& points,
                                   float density)
    : Rigidbody(dimensions, rWorld, rType, GetPolygonFixtureDef(points, dimensions.Scale), density)
    , mPolygon(device, points)
{
    CreateBody(world, type, mPolygon);
}

CircleRigidbody::CircleRigidbody(const Vortex2D::Renderer::Device& device,
                                 Vortex2D::Fluid::Dimensions dimensions,
                                 b2World& rWorld,
                                 b2BodyType rType,
                                 Vortex2D::Fluid::World& world,
                                 Vortex2D::Fluid::RigidBody::Type type,
                                 const float radius,
                                 float density)
    : Rigidbody(dimensions, rWorld, rType, GetCircleFixtureDef(radius, dimensions.Scale), density)
    , mCircle(device, radius)
{
    CreateBody(world, type, mCircle);
}

