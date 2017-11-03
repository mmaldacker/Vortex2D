//
//  Water.cpp
//  Vortex2D
//

#include <Vortex2D/Vortex2D.h>
#include <Vortex2D/Renderer/Drawable.h>
#include <Vortex2D/Engine/World.h>
#include <Vortex2D/Engine/Boundaries.h>

#include <functional>
#include <vector>
#include <memory>

extern glm::vec4 green;
extern glm::vec4 gray;
extern glm::vec4 blue;

class WaterExample : public Vortex2D::Renderer::Drawable
{
public:
    WaterExample(const Vortex2D::Renderer::Device& device,
                 const Vortex2D::Fluid::Dimensions& dimensions,
                 float dt)
        : gravity(device, dimensions.Size, {0.0f, 0.01f, 0.0f, 0.0f})
        , world(device, dimensions, dt)
        , solidPhi(device, world.SolidPhi(), green, dimensions.Scale)
        , liquidPhi(device, world.LiquidPhi(), blue, dimensions.Scale)
    {
        liquidPhi.Scale = solidPhi.Scale = glm::vec2(dimensions.Scale);

        // Add particles
        Vortex2D::Renderer::IntRectangle fluid(device, {600.0f, 200.0f}, glm::ivec4(4));
        fluid.Position = {200.0f, 100.0f};

        world.Count().View = dimensions.InvScale;
        world.Count().Record({fluid});
        world.Count().Submit();
        device.Handle().waitIdle();

        // Draw solid boundaries
        Vortex2D::Renderer::Rectangle obstacle1(device, {200.0f, 100.0f}, glm::vec4(-1.0f));
        Vortex2D::Renderer::Rectangle obstacle2(device, {200.0f, 100.0f}, glm::vec4(-1.0f));
        Vortex2D::Renderer::Rectangle area(device, dimensions.Scale * (glm::vec2(dimensions.Size) - glm::vec2(2.0f)), glm::vec4(1.0f));
        Vortex2D::Renderer::Clear clear(dimensions.Size.x, dimensions.Size.y, {-1.0f, 0.0f, 0.0f, 0.0f});

        area.Position = glm::vec2(dimensions.Scale);

        obstacle1.Position = {300.0f, 600.0f};
        obstacle1.Rotation = 45.0f;

        obstacle2.Position = {700.0f, 600.0f};
        obstacle2.Rotation = 30.0f;

        world.SolidPhi().View = dimensions.InvScale;
        world.SolidPhi().Record({clear, area, obstacle1, obstacle2});
        world.SolidPhi().Submit();

        world.SolidPhi().Reinitialise();
        device.Handle().waitIdle();

        // Set gravity
        world.Velocity().Record({gravity});
    }

    void Initialize(const Vortex2D::Renderer::RenderState& renderState) override
    {
        solidPhi.Initialize(renderState);
        liquidPhi.Initialize(renderState);
    }

    void Update(const glm::mat4& projection, const glm::mat4& view) override
    {
        solidPhi.Update(projection, view);
        liquidPhi.Update(projection, view);

        world.Velocity().Submit();
        world.SolveDynamic();
    }

    void Draw(vk::CommandBuffer commandBuffer, const Vortex2D::Renderer::RenderState& renderState) override
    {
        solidPhi.Draw(commandBuffer, renderState);
        liquidPhi.Draw(commandBuffer, renderState);
    }

private:
    Vortex2D::Renderer::Rectangle gravity;
    Vortex2D::Fluid::World world;
    Vortex2D::Fluid::DistanceField solidPhi, liquidPhi;
};
