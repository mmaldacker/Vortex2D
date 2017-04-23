//
//  RenderExample.h
//  Vortex2D
//

#include <Vortex2D/Renderer/Drawable.h>
#include <Vortex2D/Renderer/Shader.h>
#include <Vortex2D/Renderer/Shapes.h>


class RenderExample : public Vortex2D::Renderer::Drawable
{
public:
    RenderExample(const glm::vec2& size)
        : rectangle({100,100})
    {

    }

    void Render(Vortex2D::Renderer::RenderTarget & target, const glm::mat4 & transform = glm::mat4()) override
    {
        target.Render(rectangle);
    }

private:
    Vortex2D::Renderer::Rectangle rectangle;
};
