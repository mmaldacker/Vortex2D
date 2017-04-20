//
//  RenderWindow.cpp
//  Vortex2D
//

#include "RenderWindow.h"

#include <stdexcept>

namespace Vortex2D { namespace Renderer {

RenderWindow::RenderWindow(int width, int height)
    : Vortex2D::Renderer::RenderTarget(width, height)
    , mWidth(width)
    , mHeight(height)
{
}

RenderWindow::~RenderWindow()
{
}

void RenderWindow::Clear(const glm::vec4 & colour)
{
}

void RenderWindow::Render(Vortex2D::Renderer::Drawable & object, const glm::mat4 & transform)
{
    object.Render(*this, transform);
}

}}
