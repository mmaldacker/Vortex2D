//
//  RenderExample.h
//  Vortex2D
//

#include <Vortex2D/Renderer/Drawable.h>
#include <Vortex2D/Renderer/Shapes.h>

class RenderExample : public Vortex2D::Renderer::Drawable
{
public:
    RenderExample(const Vortex2D::Renderer::Device& device, const glm::vec2& size)
        : rectangle(device, {100,100}, {1.0f, 1.0f, 1.0f, 1.0f})
        , circle(device, glm::vec2(50), glm::vec4(1.0f))
    {
        rectangle.Position = {200, 200};
        rectangle.Rotation = 45.0f;

        circle.Position = {500, 500};
    }

    void Initialize(const Vortex2D::Renderer::RenderState& renderState) override
    {
        rectangle.Initialize(renderState);
        circle.Initialize(renderState);
    }

    void Update(const glm::mat4& projection, const glm::mat4& view) override
    {
        rectangle.Update(projection, view);
        circle.Update(projection, view);
    }

    void Draw(vk::CommandBuffer commandBuffer, const Vortex2D::Renderer::RenderState& renderState) override
    {
        rectangle.Draw(commandBuffer, renderState);
        circle.Draw(commandBuffer, renderState);
    }

private:
    Vortex2D::Renderer::Rectangle rectangle;
    Vortex2D::Renderer::Ellipse circle;
};
