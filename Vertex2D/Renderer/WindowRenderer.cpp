//
//  WindowRenderer.cpp
//  Vortex
//
//  Created by Maximilian Maldacker on 08/04/2014.
//
//

#include "WindowRenderer.h"

#include <glm/gtc/matrix_transform.hpp>

namespace Renderer
{

WindowRenderer::WindowRenderer(SDL_Window * window) : mWindow(window)
{
    int w,h;
    SDL_GetWindowSize(mWindow, &w, &h);

    glViewport(0, 0, w, h);

    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_BLEND);

    Ortho = glm::ortho(0.0f, (float)w, (float)h, 0.0f, -1.0f, 1.0f);
}

void WindowRenderer::SetBackgroundColour(const glm::vec4 &colour)
{
    glClearColor(colour.r, colour.g, colour.b, colour.a);
}

void WindowRenderer::AddDrawable(Drawable * drawable)
{
    mDrawables.push_back(drawable);
}

void WindowRenderer::Render()
{
    CHECK_GL_ERROR_DEBUG();

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

    for(auto drawable : mDrawables)
    {
        drawable->Render(Ortho);
    }

    SDL_GL_SwapWindow(mWindow);
}

}