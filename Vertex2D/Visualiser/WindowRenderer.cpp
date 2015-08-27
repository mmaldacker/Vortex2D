//
//  WindowRenderer.cpp
//  Vortex
//
//  Created by Maximilian Maldacker on 08/04/2014.
//
//

#include "WindowRenderer.h"

#include <glm/gtc/matrix_transform.hpp>

WindowRenderer::WindowRenderer(const glm::vec2 & size, WindowRenderer * share)
    : mWindow(glfwCreateWindow(size.x, size.y, "Window", NULL, share ? share->mWindow : NULL))
{
    if(!mWindow)
    {
        throw std::runtime_error("Error creating window");
    }

    MakeCurrent();

    glfwSwapInterval(1);

    glViewport(0, 0, size.x, size.y);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    Ortho = glm::ortho(0.0f, size.x, size.y, 0.0f, -1.0f, 1.0f);
}

WindowRenderer::~WindowRenderer()
{
    glfwDestroyWindow(mWindow);
}

void WindowRenderer::MakeCurrent()
{
    glfwMakeContextCurrent(mWindow);
}

bool WindowRenderer::ShouldClose()
{
    return glfwWindowShouldClose(mWindow);
}

void WindowRenderer::SetBackgroundColour(const glm::vec4 &colour)
{
    MakeCurrent();
    glClearColor(colour.r, colour.g, colour.b, colour.a);
}

void WindowRenderer::AddDrawable(Renderer::Drawable & drawable)
{
    mDrawables.push_back(&drawable);
}

void WindowRenderer::Render()
{
    MakeCurrent();

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

    for(auto drawable : mDrawables)
    {
        drawable->Render(Ortho);
    }

    CHECK_GL_ERROR_DEBUG();

    glfwSwapBuffers(mWindow);
}
