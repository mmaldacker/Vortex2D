//
//  WindmillWaterExample.h
//  Vortex2D
//

#include <Vortex2D/Vortex2D.h>

#include "Runner.h"
#include "Rigidbody.h"

#include <functional>
#include <vector>
#include <memory>
#include <cmath>
#include <glm/gtc/constants.hpp>

extern glm::vec4 red;
extern glm::vec4 blue;

class Watermill
{
public:
    Watermill(const Vortex2D::Renderer::Device& device,
             b2World& rWorld,
             Vortex2D::Fluid::World& world)
        : mWindmillTexture(device, 600, 600, vk::Format::eR32Sfloat)
        , mWindmill(device, mWindmillTexture)
        , mRigidbody(world.CreateRigidbody(Vortex2D::Fluid::RigidBody::Type::eStrong, mWindmill, {300.0f, 300.0f}))
    {
        glm::vec2 centre(300.0f, 300.0f);
        mRigidbody->Anchor = centre;

        { // create centre body
            b2BodyDef def;
            def.type = b2_staticBody;

            mCentre = rWorld.CreateBody(&def);

            b2CircleShape circleShape;
            circleShape.m_radius = 1.0f / box2dScale;

            b2FixtureDef fixture;
            fixture.density = 1.0f;
            fixture.shape = &circleShape;

            mCentre->CreateFixture(&fixture);
        }

        { // create wings body and joint
            b2BodyDef def;
            def.type =  b2_dynamicBody;
            def.angularDamping = 3.0f;
            mWings = rWorld.CreateBody(&def);

            b2RevoluteJointDef joint;
            joint.bodyA = mCentre;
            joint.bodyB = mWings;
            joint.collideConnected = false;

            rWorld.CreateJoint(&joint);
        }

        // build wings
        mWindmillTexture.Record({Vortex2D::Fluid::BoundariesClear}).Submit().Wait();

        int n = 6;
        float dist = 120.0f;
        glm::vec2 size(250.0f, 30.0f);

        for (int i = 0; i < n; i++)
        {
            float angle = i * 2.0f * glm::pi<float>() / n;
            float x = dist * std::cos(angle);
            float y = dist * std::sin(angle);

            b2PolygonShape wing;
            wing.SetAsBox(size.x / (2.0f * box2dScale), size.y / (2.0f * box2dScale),
                          b2Vec2(x / box2dScale, y / box2dScale),
                          angle);

            b2FixtureDef fixtureDef;
            fixtureDef.shape = &wing;
            fixtureDef.density = 1.0f;

            mWings->CreateFixture(&fixtureDef);

            Vortex2D::Fluid::Rectangle fluidWing(device, size);
            fluidWing.Anchor = size / glm::vec2(2.0f);
            fluidWing.Position = glm::vec2(x, y) + centre;
            fluidWing.Rotation = glm::degrees(angle);
            mWindmillTexture.Record({fluidWing}, Vortex2D::Fluid::UnionBlend).Submit().Wait();
        }
    }

    void SetTransform(const glm::vec2& pos, float angle)
    {
        mRigidbody->Position = pos;
        mRigidbody->Rotation = angle;
        mCentre->SetTransform({pos.x / box2dScale, pos.y / box2dScale}, angle);
        mWings->SetTransform({pos.x / box2dScale, pos.y / box2dScale}, angle);
    }

    void Update()
    {
        auto pos = mWings->GetPosition();
        mRigidbody->Position = {pos.x * box2dScale, pos.y * box2dScale};
        mRigidbody->Rotation = glm::degrees(mWings->GetAngle());
        mRigidbody->UpdatePosition();

        glm::vec2 vel = {mWings->GetLinearVelocity().x, mWings->GetLinearVelocity().y};
        float angularVelocity = mWings->GetAngularVelocity();
        mRigidbody->SetVelocities(vel * box2dScale, angularVelocity);

        auto force = mRigidbody->GetForces();
        float scale = 1.0f / box2dScale;
        b2Vec2 b2Force = {force.velocity.x * scale, force.velocity.y * scale};
        mWings->ApplyForceToCenter(b2Force, true);
        mWings->ApplyTorque(force.angular_velocity, true);
    }

private:
    float mScale;
    Vortex2D::Renderer::RenderTexture mWindmillTexture;
    Vortex2D::Renderer::Sprite mWindmill;
    Vortex2D::Fluid::RigidBody* mRigidbody;
    b2Body* mCentre;
    b2Body* mWings;
};

class WatermillExample : public Runner
{
    const float gravityForce = 300.0f;

public:
    WatermillExample(const Vortex2D::Renderer::Device& device,
                         const Vortex2D::Fluid::Dimensions& dimensions,
                         float dt)
        : delta(dt)
        , waterSource(device, {50.0f, 50.0f})
        , waterForce(device, {50.0f, 50.0f})
        , gravity(device, glm::vec2(1024.0f, 1024.0f))
        , world(device, dimensions, dt)
        , solidPhi(world.SolidDistanceField())
        , liquidPhi(world.LiquidDistanceField())
        , rWorld(b2Vec2(0.0f, gravityForce / box2dScale))
        , left(device, rWorld, b2_staticBody, world, Vortex2D::Fluid::RigidBody::Type::eStatic, {200.0f, 20.0f})
        , bottom(device, rWorld, b2_staticBody, world, Vortex2D::Fluid::RigidBody::Type::eStatic, {1000.0f, 20.0f})
        , windmill(device, rWorld, world)
    {
        liquidPhi.Scale = solidPhi.Scale = glm::vec2(dimensions.Scale);
        gravity.Colour = glm::vec4(0.0f, dt * gravityForce, 0.0f, 0.0f);

        solidPhi.Colour = red;
        liquidPhi.Colour = blue;
    }

    void Init(const Vortex2D::Renderer::Device& device,
              Vortex2D::Renderer::RenderTarget& renderTarget) override
    {

        // Add particles
        waterSource.Position = {20.0f, 100.0f};
        waterSource.Colour = glm::vec4(4);

        // Add force
        waterForce.Position = {20.0f, 100.0f};
        waterForce.Colour = glm::vec4(0.01f, 0.0f, 0.0f, 0.0f);

        sourceRender = world.RecordParticleCount({waterSource});

        // Draw boundaries
        left.SetTransform({200.0f, 350.0f}, 60.0f);
        left.Update();

        bottom.SetTransform({10.0f, 1000.0f}, 0.0f);
        bottom.Update();

        windmill.SetTransform({650.0f, 650.0f}, 25.0f);

        // Set gravity
        velocityRender = world.RecordVelocity({gravity, waterForce});

        Vortex2D::Renderer::ColorBlendState blendState;
        blendState.ColorBlend
                .setBlendEnable(true)
                .setAlphaBlendOp(vk::BlendOp::eAdd)
                .setColorBlendOp(vk::BlendOp::eAdd)
                .setSrcColorBlendFactor(vk::BlendFactor::eSrcAlpha)
                .setSrcAlphaBlendFactor(vk::BlendFactor::eOne)
                .setDstColorBlendFactor(vk::BlendFactor::eOneMinusSrcAlpha)
                .setDstAlphaBlendFactor(vk::BlendFactor::eZero);

        windowRender = renderTarget.Record({liquidPhi, solidPhi}, blendState);
    }

    void Step() override
    {
        windmill.Update();

        sourceRender.Submit();
        world.SubmitVelocity(velocityRender);
        world.Solve();

        const int velocityStep = 8;
        const int positionStep = 3;
        rWorld.Step(delta, velocityStep, positionStep);

        windowRender.Submit();
    }

private:
    float delta;
    Vortex2D::Renderer::IntRectangle waterSource;
    Vortex2D::Renderer::Rectangle waterForce;
    Vortex2D::Renderer::Rectangle gravity;
    Vortex2D::Fluid::WaterWorld world;
    Vortex2D::Fluid::DistanceField solidPhi, liquidPhi;
    Vortex2D::Renderer::RenderCommand sourceRender, velocityRender, windowRender;

    b2World rWorld;
    BoxRigidbody left, bottom;
    Watermill windmill;
};
