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
              const glm::ivec2& size,
              b2World& rWorld,
              Vortex2D::Fluid::World& world)
        : mWatermillTexture(device, 150.0f, 150.0f, vk::Format::eR32Sfloat)
        , mWatermill(device, mWatermillTexture)
        , mRigidbody(device, size, mWatermill, Vortex2D::Fluid::RigidBody::Type::eStrong)
    {
        { // create centre body
            b2BodyDef def;
            def.type = b2_staticBody;

            mCentre = rWorld.CreateBody(&def);

            b2CircleShape circleShape;
            circleShape.m_radius = 1.0f;

            b2FixtureDef fixture;
            fixture.density = 1.0f;
            fixture.shape = &circleShape;

            mCentre->CreateFixture(&fixture);
        }

        { // create wings body and joint
            b2BodyDef def;
            def.type =  b2_dynamicBody;
            def.angularDamping = 3.0f;
            mRigidbody.mBody = rWorld.CreateBody(&def);

            b2RevoluteJointDef joint;
            joint.bodyA = mCentre;
            joint.bodyB = mRigidbody.mBody;
            joint.collideConnected = false;

            rWorld.CreateJoint(&joint);
        }

        // build wings
        mWatermillTexture.Record({Vortex2D::Fluid::BoundariesClear}).Submit().Wait();

        int n = 6;
        float dist = 30.0f;
        glm::vec2 wingSize(75.0f, 8.0f);
        glm::vec2 centre(75.0f, 75.0f);

        for (int i = 0; i < n; i++)
        {
            float angle = i * 2.0f * glm::pi<float>() / n;
            float x = dist * std::cos(angle);
            float y = dist * std::sin(angle);

            b2PolygonShape wing;
            wing.SetAsBox(wingSize.x / 2.0f, wingSize.y / 2.0f,
                          b2Vec2(x, y),
                          angle);

            b2FixtureDef fixtureDef;
            fixtureDef.shape = &wing;
            fixtureDef.density = 1.0f;

            mRigidbody.mBody->CreateFixture(&fixtureDef);

            Vortex2D::Fluid::Rectangle fluidWing(device, wingSize);
            fluidWing.Anchor = wingSize / glm::vec2(2.0f);
            fluidWing.Position = glm::vec2(x, y) + centre;
            fluidWing.Rotation = glm::degrees(angle);
            mWatermillTexture.Record({fluidWing}, Vortex2D::Fluid::UnionBlend).Submit().Wait();
        }

        mRigidbody.Anchor = centre;
        mRigidbody.SetMassData(mRigidbody.mBody->GetMass(), mRigidbody.mBody->GetInertia());
        world.AddRigidbody(mRigidbody);
    }

    void SetTransform(const glm::vec2& pos, float angle)
    {
        mCentre->SetTransform({pos.x, pos.y}, angle);
        mRigidbody.SetTransform(pos, angle);
    }

private:
    Vortex2D::Renderer::RenderTexture mWatermillTexture;
    Vortex2D::Renderer::Sprite mWatermill;
    Box2DRigidbody mRigidbody;
    b2Body* mCentre;
};

class WatermillExample : public Runner
{
    const float gravityForce = 100.0f;

public:
    WatermillExample(const Vortex2D::Renderer::Device& device,
                         const glm::ivec2& size,
                         float dt)
        : waterSource(device, {25.0, 25.0f})
        , waterForce(device, {25.0f, 25.0f})
        , gravity(device, glm::vec2(256.0f, 256.0f))
        , world(device, size, dt, 2)
        , solidPhi(world.SolidDistanceField())
        , liquidPhi(world.LiquidDistanceField())
        , rWorld(b2Vec2(0.0f, gravityForce))
        , solver(rWorld)
        , left(device, size, rWorld, b2_staticBody, world, Vortex2D::Fluid::RigidBody::Type::eStatic, {50.0f, 5.0f})
        , bottom(device, size, rWorld, b2_staticBody, world, Vortex2D::Fluid::RigidBody::Type::eStatic, {250.0f, 5.0f})
        , watermill(device, size, rWorld, world)
    {
        world.AttachRigidBodySolver(solver);

        gravity.Colour = glm::vec4(0.0f, dt * gravityForce, 0.0f, 0.0f);

        solidPhi.Colour = red;
        liquidPhi.Colour = blue;
    }

    void Init(const Vortex2D::Renderer::Device& device,
              Vortex2D::Renderer::RenderTarget& renderTarget) override
    {

        // Add particles
        waterSource.Position = {15.0f, 25.0f};
        waterSource.Colour = glm::vec4(4);

        // Add force
        waterForce.Position = {5.0f, 25.0f};
        waterForce.Colour = glm::vec4(20.0f, 0.0f, 0.0f, 0.0f);

        sourceRender = world.RecordParticleCount({waterSource});

        // Draw boundaries
        left.mRigidbody.SetTransform({50.0f, 100.0f}, 60.0f);
        bottom.mRigidbody.SetTransform({5.0f, 250.0f}, 0.0f);

        watermill.SetTransform({150.0f, 180.0f}, 25.0f);

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
        sourceRender.Submit();
        world.SubmitVelocity(velocityRender);
        auto params = Vortex2D::Fluid::FixedParams(12);
        world.Step(params);
        windowRender.Submit();
    }

private:
    Vortex2D::Renderer::IntRectangle waterSource;
    Vortex2D::Renderer::Rectangle waterForce;
    Vortex2D::Renderer::Rectangle gravity;
    Vortex2D::Fluid::WaterWorld world;
    Vortex2D::Fluid::DistanceField solidPhi, liquidPhi;
    Vortex2D::Renderer::RenderCommand sourceRender, velocityRender, windowRender;

    b2World rWorld;

    Box2DSolver solver;
    RectangleRigidbody left, bottom;
    Watermill watermill;
};
