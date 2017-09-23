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
        : gravity(device, dimensions.Size, {0.0f, -0.5f, 0.0f, 0.0f})
        , world(device, dimensions, dt)
        , solidPhi(device, world.SolidPhi(), green, dimensions.Scale)
        , liquidPhi(device, world.LiquidPhi(), blue, dimensions.Scale, true)
        , particleCloud(device, world.Particles(), 0, blue)
    {
        // TODO should set the view and not the scale
        particleCloud.Scale = liquidPhi.Scale = solidPhi.Scale = (glm::vec2)dimensions.Scale;

        // Add particles
        Vortex2D::Renderer::IntRectangle fluid(device, {600.0f, 200.0f}, glm::ivec4(4));
        fluid.Position = {200.0f, 100.0f};

        fluid.Initialize(world.Count());
        fluid.Update(world.Count().Orth, dimensions.InvScale);

        world.Count().Record([&](vk::CommandBuffer commandBuffer)
        {
            fluid.Draw(commandBuffer, {world.Count()});
        });
        world.Count().Submit();
        device.Handle().waitIdle();

        world.Count().Scan();
        particleCloud.SetNumParticles(world.Count().GetCount());

        // Draw solid boundaries
        Vortex2D::Renderer::Rectangle obstacle1(device, {200.0f, 100.0f}, glm::vec4(-1.0f));
        Vortex2D::Renderer::Rectangle obstacle2(device, {200.0f, 100.0f}, glm::vec4(-1.0f));
        Vortex2D::Renderer::Rectangle area(device, dimensions.Scale * glm::vec2(dimensions.Size) - glm::vec2(8.0f), glm::vec4(1.0f));

        area.Position = glm::vec2(4.0f);

        obstacle1.Position = {300.0f, 600.0f};
        obstacle1.Rotation = 45.0f;

        obstacle2.Position = {700.0f, 600.0f};
        obstacle2.Rotation = 30.0f;

        area.Initialize(world.SolidPhi());
        obstacle1.Initialize(world.SolidPhi());
        obstacle2.Initialize(world.SolidPhi());

        area.Update(world.SolidPhi().Orth, dimensions.InvScale);
        obstacle1.Update(world.SolidPhi().Orth, dimensions.InvScale);
        obstacle2.Update(world.SolidPhi().Orth, dimensions.InvScale);

        world.SolidPhi().Record([&](vk::CommandBuffer commandBuffer)
        {
            Vortex2D::Renderer::Clear(dimensions.Size.x, dimensions.Size.y, {-1.0f, 0.0f, 0.0f, 0.0f}).Draw(commandBuffer);
            area.Draw(commandBuffer, world.SolidPhi());
            obstacle1.Draw(commandBuffer, world.SolidPhi());
            obstacle2.Draw(commandBuffer, world.SolidPhi());
        });
        world.SolidPhi().Submit();

        world.SolidPhi().Reinitialise();
        device.Handle().waitIdle();
    }

    void Initialize(const Vortex2D::Renderer::RenderState& renderState) override
    {
        solidPhi.Initialize(renderState);
        liquidPhi.Initialize(renderState);
        particleCloud.Initialize(renderState);
    }

    void Update(const glm::mat4& projection, const glm::mat4& view) override
    {
        solidPhi.Update(projection, view);
        liquidPhi.Update(projection, view);
        particleCloud.Update(projection, view);
    }

    void Draw(vk::CommandBuffer commandBuffer, const Vortex2D::Renderer::RenderState& renderState) override
    {
        solidPhi.Draw(commandBuffer, renderState);
        liquidPhi.Draw(commandBuffer, renderState);
        particleCloud.Draw(commandBuffer, renderState);
    }

    std::vector<glm::vec2> GenerateParticles(const glm::vec2& pos, const glm::vec2& size)
    {
        std::vector<glm::vec2> particles;
        for (int i = pos.x; i < pos.x+size.x; i+=4)
        {
            for (int j = pos.y; j < pos.y+size.y; j+=4)
            {
                particles.push_back(glm::vec2(i, j));
            }
        }

        return particles;
    }

private:
    Vortex2D::Renderer::Rectangle gravity;
    Vortex2D::Fluid::World world;
    Vortex2D::Fluid::DistanceField solidPhi, liquidPhi;
    Vortex2D::Fluid::ParticleCloud particleCloud;
};
