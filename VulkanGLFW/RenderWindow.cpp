//
//  RenderWindow.cpp
//  Vortex2D
//

#include "RenderWindow.h"

#include <stdexcept>

RenderWindow::RenderWindow(int width, int height, const std::string & name, RenderWindow * share)
    : Vortex2D::Renderer::RenderTarget(width, height)
    , mWidth(width)
    , mHeight(height)
    , mWindow(glfwCreateWindow(width, height, name.c_str(), nullptr, share ? share->mWindow : nullptr))
{
    if (!mWindow)
        throw std::runtime_error("glfwCreateWindow failed");

}

RenderWindow::~RenderWindow()
{
    glfwDestroyWindow(mWindow);
}

void RenderWindow::MakeCurrent()
{
    glfwMakeContextCurrent(mWindow);
}

void RenderWindow::Clear(const glm::vec4 & colour)
{
}

void RenderWindow::Render(Vortex2D::Renderer::Drawable & object, const glm::mat4 & transform)
{
    object.Render(*this, transform);
}

void RenderWindow::Display()
{
    glfwSwapBuffers(mWindow);
}

bool RenderWindow::ShouldClose()
{
    return glfwWindowShouldClose(mWindow);
}

void RenderWindow::SetKeyCallback(GLFWkeyfun cbfun)
{
    glfwSetKeyCallback(mWindow, cbfun);
}
