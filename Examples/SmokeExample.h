//
//  Smoke.cpp
//  Vortex2D
//

#include <Vortex2D/Vortex2D.h>
#include <Vortex2D/Renderer/Drawable.h>
#include <Vortex2D/Engine/World.h>
#include <Vortex2D/Renderer/RenderTexture.h>
#include <Vortex2D/Renderer/Sprite.h>

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
        , force2(device, glm::vec2(20.0f), {0.0f, 0.5f, 0.0f, 0.0f})
        , density(device, dimensions.Size.x, dimensions.Size.y, vk::Format::eB8G8R8A8Unorm)
        , densitySprite(device, density)
        , world(device, dimensions, dt)
    {
        // TODO should set the view and not the scale
        densitySprite.Scale = (glm::vec2)dimensions.Scale;

        source1.Position = force1.Position = {250.0f, 100.0f};
        source2.Position = force2.Position = {750.0f, 100.0f};

        // TODO could be shorter
        Vortex2D::Renderer::Rectangle area(device, dimensions.Size - glm::ivec2(2.0f), glm::vec4(1.0f));
        area.Position = glm::vec2(1.0f);

        Vortex2D::Renderer::Clear clear(dimensions.Size.x, dimensions.Size.y, {-1.0f, 0.0f, 0.0f, 0.0f});

        world.LiquidPhi().Record({clear, area});
        world.LiquidPhi().Submit();
        device.Handle().waitIdle(); // needed since it needs to be finished before the area object is destroyed

        source1.Update(density.Orth, dimensions.InvScale);
        source2.Update(density.Orth, dimensions.InvScale);

        force1.Update(world.Velocity().Orth, dimensions.InvScale);
        force2.Update(world.Velocity().Orth, dimensions.InvScale);

        world.InitField(density);

        world.Velocity().Record({force1, force2});
        density.Record({source1, source2});
    }

    void Initialize(const Vortex2D::Renderer::RenderState& renderState) override
    {
        densitySprite.Initialize(renderState);
    }

    void Update(const glm::mat4& projection, const glm::mat4& view) override
    {
        densitySprite.Update(projection, view);

        world.Velocity().Submit();
        density.Submit();

        world.SolveStatic();
    }

    void Draw(vk::CommandBuffer commandBuffer, const Vortex2D::Renderer::RenderState& renderState) override
    {
        densitySprite.Draw(commandBuffer, renderState);
    }

private:
    Vortex2D::Renderer::Ellipse source1, source2;
    Vortex2D::Renderer::Ellipse force1, force2;
    Vortex2D::Renderer::RenderTexture density;
    Vortex2D::Renderer::Sprite densitySprite;
    Vortex2D::Fluid::World world;
};
