//
//  ObstacleSmoke.cpp
//  Vortex2D
//

#include <Vortex2D/Vortex2D.h>
#include <Vortex2D/Renderer/Drawable.h>
#include <Vortex2D/Engine/World.h>
#include <Vortex2D/Renderer/RenderTexture.h>
#include <Vortex2D/Renderer/Sprite.h>
#include <Vortex2D/Engine/Boundaries.h>

#include <Box2D/Box2D.h>

#include <glm/trigonometric.hpp>

#include <functional>
#include <vector>
#include <memory>

extern glm::vec4 green;
extern glm::vec4 gray;
extern glm::vec4 blue;

const float box2dScale = 32.0f;

class ObstacleSmokeExample : public Vortex2D::Renderer::Drawable
{
public:
    ObstacleSmokeExample(const Vortex2D::Renderer::Device& device,
                         const Vortex2D::Fluid::Dimensions& dimensions,
                         float dt)
        : delta(dt)
        , dimensions(dimensions)
        , density(device, dimensions.Size.x, dimensions.Size.y, vk::Format::eB8G8R8A8Unorm)
        , densitySprite(device, density)
        , world(device, dimensions, dt)
        , solidPhi(device, world.SolidPhi(), green, dimensions.Scale)
        , area(device, {960.0f, 960.0f}, true)
        , obstacle1(device, {100.0f, 100.0f})
        , obstacle2(device, {100.0f, 100.0f})
        , rWorld({0.0f, 10.0f})
    {
        // TODO should set the view and not the scale
        solidPhi.Scale = densitySprite.Scale = (glm::vec2)dimensions.Scale;

        world.InitField(density);

        // Draw density
        Vortex2D::Renderer::Rectangle source(device, {800.0f, 400.0f}, gray);
        source.Position = {100.0f, 500.0f};
        source.Update(density.Orth, dimensions.InvScale);

        density.Record({source});
        density.Submit();
        device.Handle().waitIdle();

        // Draw liquid boundaries
        Vortex2D::Renderer::Clear clear(dimensions.Size.x, dimensions.Size.y, {-1.0f, 0.0f, 0.0f, 0.0f});
        world.LiquidPhi().Record({clear});
        world.LiquidPhi().Submit();
        device.Handle().waitIdle();

        // Draw solid boundaries
        area.Position = {20.0f, 20.0f};

        b2PolygonShape boxShape;
        boxShape.SetAsBox(50.0f / box2dScale, 50.0f / box2dScale);

        b2FixtureDef fixtureDef;
        fixtureDef.shape = &boxShape;
        fixtureDef.density = 1.0f;
        fixtureDef.restitution = 0.8f;

        // First body
        b2BodyDef def1;
        def1.type = b2_dynamicBody;
        def1.position = {200.0f / box2dScale, 200.0f / box2dScale};

        rObstacle1 = rWorld.CreateBody(&def1);
        rObstacle1->CreateFixture(&fixtureDef);
        rObstacle1->ApplyAngularImpulse(5.0f, true);
        rObstacle1->ApplyForceToCenter({-1000.0f, 0.0f}, true);

        obstacle1.Anchor = {50.0f, 50.0f};

        // Second body
        b2BodyDef def2;
        def2.type = b2_dynamicBody;
        def2.position = {600.0f / box2dScale, 300.0f / box2dScale};

        rObstacle2 = rWorld.CreateBody(&def2);
        rObstacle2->CreateFixture(&fixtureDef);
        rObstacle2->ApplyAngularImpulse(-10.0f, true);
        rObstacle2->ApplyForceToCenter({1000.0f, 0.0f}, true);

        obstacle2.Anchor = {50.0f, 50.0f};

        // Borders
        b2BodyDef border;
        b2EdgeShape line;

        line.Set({20.0f / box2dScale, 980.0f / box2dScale}, {980.0f / box2dScale, 980.0f / box2dScale});
        auto* bottom = rWorld.CreateBody(&border);
        bottom->CreateFixture(&line, 0.0f);

        line.Set({20.0f / box2dScale, 20.0f / box2dScale}, {20.0f / box2dScale, 980.0f / box2dScale});
        auto* left = rWorld.CreateBody(&border);
        left->CreateFixture(&line, 0.0f);

        line.Set({980.0f / box2dScale, 20.0f / box2dScale}, {980.0f / box2dScale, 980.0f / box2dScale});
        auto* right = rWorld.CreateBody(&border);
        right->CreateFixture(&line, 0.0f);

        world.SolidPhi().DrawSignedObject({area, obstacle1, obstacle2});
    }

    void Initialize(const Vortex2D::Renderer::RenderState& renderState) override
    {
        densitySprite.Initialize(renderState);
        solidPhi.Initialize(renderState);
    }

    void Update(const glm::mat4& projection, const glm::mat4& view) override
    {
        auto pos1 = rObstacle1->GetPosition();
        obstacle1.Position = {pos1.x * box2dScale, pos1.y * box2dScale};
        obstacle1.Rotation = glm::degrees(rObstacle1->GetAngle());

        auto pos2 = rObstacle2->GetPosition();
        obstacle2.Position = {pos2.x * box2dScale, pos2.y * box2dScale};
        obstacle2.Rotation = glm::degrees(rObstacle2->GetAngle());

        obstacle1.Update(dimensions.InvScale);
        obstacle2.Update(dimensions.InvScale);
        area.Update(dimensions.InvScale);

        densitySprite.Update(projection, view);
        solidPhi.Update(projection, view);

        world.SolidPhi().SubmitSignedBoject();

        world.SolveStatic();

        const int velocityStep = 8;
        const int positionStep = 3;
        rWorld.Step(delta, velocityStep, positionStep);
    }

    void Draw(vk::CommandBuffer commandBuffer, const Vortex2D::Renderer::RenderState& renderState) override
    {
        densitySprite.Draw(commandBuffer, renderState);
        solidPhi.Draw(commandBuffer, renderState);
    }

private:
    float delta;
    Vortex2D::Fluid::Dimensions dimensions;
    Vortex2D::Renderer::RenderTexture density;
    Vortex2D::Renderer::Sprite densitySprite;
    Vortex2D::Fluid::World world;
    Vortex2D::Fluid::DistanceField solidPhi;

    Vortex2D::Fluid::Rectangle area;
    Vortex2D::Fluid::Rectangle obstacle1;
    Vortex2D::Fluid::Rectangle obstacle2;

    b2Body* rObstacle1;
    b2Body* rObstacle2;

    b2World rWorld;
};
