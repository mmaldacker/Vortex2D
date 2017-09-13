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

#include <functional>
#include <vector>
#include <memory>

extern glm::vec4 green;
extern glm::vec4 gray;
extern glm::vec4 blue;

class ObstacleSmokeExample : public Vortex2D::Renderer::Drawable
{
public:
    ObstacleSmokeExample(const Vortex2D::Renderer::Device& device,
                         const Vortex2D::Fluid::Dimensions& dimensions,
                         float dt)
        : source(device, glm::vec2(20.0f), gray)
        , force(device, glm::vec2(20.0f), {0.0f, 0.1f, 0.0f, 0.0f})
        , density(device, dimensions.Size.x, dimensions.Size.y, vk::Format::eB8G8R8A8Unorm)
        , densitySprite(device, density)
        , world(device, dimensions, dt)
        , solidPhi(device, world.SolidPhi(), dimensions.Scale)
    {
        solidPhi.Scale = densitySprite.Scale = (glm::vec2)dimensions.Scale;

        force.Position = source.Position = {200.0f, 100.0f};

        source.Initialize({density});
        force.Initialize(world.Velocity());

        source.Update(density.Orth, dimensions.InvScale);
        force.Update(world.Velocity().Orth, dimensions.InvScale);

        world.InitField(density);
        world.Velocity().Record([&](vk::CommandBuffer commandBuffer)
        {
            force.Draw(commandBuffer, {world.Velocity()});
        });

        density.Record([&](vk::CommandBuffer commandBuffer)
        {
            source.Draw(commandBuffer, {density});
        });

        // Draw liquid boundaries
        world.LiquidPhi().Record([&](vk::CommandBuffer commandBuffer)
        {
            Vortex2D::Renderer::Clear(dimensions.Size.x, dimensions.Size.y, {-1.0f, 0.0f, 0.0f, 0.0f})
                    .Draw(commandBuffer);
        });
        world.LiquidPhi().Submit();
        device.Handle().waitIdle();

        // TODO could be shorter
        // Draw solid boundaries
        Vortex2D::Fluid::Rectangle obstacle(device, {200.0f, 100.0f});

        obstacle.Position = {200.0f, 300.0f};
        obstacle.Rotation = 45.0f;

        obstacle.Initialize(world.SolidPhi());
        obstacle.Update(dimensions.InvScale);
        Vortex2D::Renderer::ExecuteCommand(device, [&](vk::CommandBuffer commandBuffer)
        {
            world.SolidPhi().Clear(commandBuffer, {-1000.0f, 0.0f, 0.0f, 0.0f});
            obstacle.Draw(commandBuffer, world.SolidPhi());
        });
    }

    void Initialize(const Vortex2D::Renderer::RenderState& renderState) override
    {
        densitySprite.Initialize(renderState);
        solidPhi.Initialize(renderState);
    }

    void Update(const glm::mat4& projection, const glm::mat4& view) override
    {
        densitySprite.Update(projection, view);
        solidPhi.Update(projection, view);

        world.Velocity().Submit();
        density.Submit();

        world.SolveStatic();
    }

    void Draw(vk::CommandBuffer commandBuffer, const Vortex2D::Renderer::RenderState& renderState) override
    {
        densitySprite.Draw(commandBuffer, renderState);
        solidPhi.Draw(commandBuffer, renderState);
    }

private:
    Vortex2D::Renderer::Ellipse source, force;
    Vortex2D::Renderer::RenderTexture density;
    Vortex2D::Renderer::Sprite densitySprite;
    Vortex2D::Fluid::World world;
    Vortex2D::Fluid::DistanceField solidPhi;
};
