//
//  RenderWindow.h
//  Vortex2D
//

#ifndef RenderWindow_h
#define RenderWindow_h

#include <Vortex2D/Renderer/RenderTarget.h>
#include <string>

namespace Vortex2D { namespace Renderer {


class RenderWindow : public Vortex2D::Renderer::RenderTarget
{
public:
    RenderWindow(vk::Device device, int width, int height);

    virtual ~RenderWindow();

    void Clear(const glm::vec4 & colour);
    void Render(Vortex2D::Renderer::Drawable & object, const glm::mat4 & transform);

private:
    int mWidth, mHeight;
};

}}

#endif
