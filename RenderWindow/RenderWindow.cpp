//
//  RenderWindow.cpp
//  Vortex2D
//

#include "RenderWindow.h"
#include <glad/glad.h>
#include <stdexcept>

RenderWindow::RenderWindow(int width, int height, const std::string & name, RenderWindow * share)
    : Vortex2D::Renderer::RenderTarget(width, height)
    , mWindow(glfwCreateWindow(width, height, name.c_str(), nullptr, share ? share->mWindow : nullptr))
{
    if (!mWindow)
        throw std::runtime_error("glfwCreateWindow failed");

    glfwMakeContextCurrent(mWindow);
    gladLoadGLLoader((GLADloadproc) glfwGetProcAddress);

    glViewport(0, 0, width, height);
    glfwSwapInterval(1);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDisable(GL_DEPTH_TEST);
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
    glClearColor(colour.r, colour.g, colour.b, colour.a);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
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
