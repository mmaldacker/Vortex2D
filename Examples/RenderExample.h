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
    {
        rectangle.Position = {200, 200};
    }

    void Initialize(const Vortex2D::Renderer::RenderState& renderState) override
    {
        rectangle.Initialize(renderState);
    }

    void Update(const glm::mat4& model, const glm::mat4& view) override
    {
        rectangle.Update(model, view);
    }

    void Draw(vk::CommandBuffer commandBuffer, const Vortex2D::Renderer::RenderState& renderState) override
    {
        rectangle.Draw(commandBuffer, renderState);
    }

private:
    Vortex2D::Renderer::Rectangle rectangle;
};
