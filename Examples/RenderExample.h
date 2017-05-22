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

    void Create(Vortex2D::Renderer::RenderTarget& renderTarget) override
    {
        rectangle.Create(renderTarget);
    }


    void Draw(vk::CommandBuffer commandBuffer, vk::RenderPass renderPass) override
    {
        rectangle.Draw(commandBuffer, renderPass);
    }

private:
    Vortex2D::Renderer::Rectangle rectangle;
};
