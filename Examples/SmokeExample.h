//
//  Smoke.cpp
//  Vortex2D
//

#include <Vortex2D/Vortex2D.h>
#include <Vortex2D/Renderer/Drawable.h>
#include <Vortex2D/Engine/World.h>
#include <Vortex2D/Engine/Density.h>
#include <Vortex2D/Renderer/RenderTexture.h>
#include <Vortex2D/Renderer/Sprite.h>
#include <Vortex2D/Engine/Boundaries.h>

#include <functional>
#include <vector>
#include <memory>

extern glm::vec4 green;
extern glm::vec4 gray;

class SmokeExample : public Vortex2D::Renderer::Drawable
{
public:
    SmokeExample(const Vortex2D::Renderer::Device& device,
                 const Vortex2D::Fluid::Dimensions& dimensions,
                 float dt)
        : source1(device, glm::vec2(20.0f), gray)
        , source2(device, glm::vec2(20.0f), gray)
        , force1(device, glm::vec2(20.0f), {0.0f, 0.5f, 0.0f, 0.0f})
        , force2(device, glm::vec2(20.0f), {0.0f, -0.5f, 0.0f, 0.0f})
        , density(device, dimensions.Size, vk::Format::eB8G8R8A8Unorm)
        , world(device, dimensions, dt)
        , solidPhi(device, world.SolidPhi(), green, dimensions.Scale)
    {
        solidPhi.Scale = density.Scale = (glm::vec2)dimensions.Scale;

        source1.Position = force1.Position = {250.0f, 100.0f};
        source2.Position = force2.Position = {750.0f, 900.0f};

        // Draw liquid boundaries
        Vortex2D::Renderer::Rectangle area(device, dimensions.Size - glm::ivec2(2.0f), glm::vec4(-1.0f));
        Vortex2D::Renderer::Clear clearLiquid({1.0f, 0.0f, 0.0f, 0.0f});

        area.Position = glm::vec2(1.0f);

        world.LiquidPhi().Record({clearLiquid, area}).Submit();

        // Draw solid boundaries
        Vortex2D::Renderer::Clear clearObstacles({1000.0, 0.0f, 0.0f, 0.0f});
        Vortex2D::Fluid::Circle obstacle1(device, 50.0f);
        Vortex2D::Fluid::Circle obstacle2(device, 50.0f);

        obstacle1.Position = {250.0f, 400.0f};
        obstacle2.Position = {750.0f, 600.0f};

        world.SolidPhi().View = dimensions.InvScale;
        world.SolidPhi().Record({clearObstacles, obstacle1, obstacle2}, Vortex2D::Fluid::UnionBlend).Submit();

        // Draw sources and forces
        world.InitField(density);

        world.Velocity().View = dimensions.InvScale;
        density.View = dimensions.InvScale;

        velocityRender = world.Velocity().Record({force1, force2});
        densityRender = density.Record({source1, source2});

        // wait for all drawing to finish
        device.Handle().waitIdle();
    }

    void Initialize(const Vortex2D::Renderer::RenderState& renderState) override
    {
        density.Initialize(renderState);
        solidPhi.Initialize(renderState);
    }

    void Update(const glm::mat4& projection, const glm::mat4& view) override
    {
        density.Update(projection, view);
        solidPhi.Update(projection, view);

        velocityRender.Submit();
        densityRender.Submit();

        world.SolveStatic();
    }

    void Draw(vk::CommandBuffer commandBuffer, const Vortex2D::Renderer::RenderState& renderState) override
    {
        density.Draw(commandBuffer, renderState);
        solidPhi.Draw(commandBuffer, renderState);
    }

private:
    Vortex2D::Renderer::Ellipse source1, source2;
    Vortex2D::Renderer::Ellipse force1, force2;
    Vortex2D::Fluid::Density density;
    Vortex2D::Fluid::World world;
    Vortex2D::Fluid::DistanceField solidPhi;
    Vortex2D::Renderer::RenderCommand velocityRender, densityRender;
};
