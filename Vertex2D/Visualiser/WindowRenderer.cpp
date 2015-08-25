//
//  WindowRenderer.cpp
//  Vortex
//
//  Created by Maximilian Maldacker on 08/04/2014.
//
//

#include "WindowRenderer.h"

#include <glm/gtc/matrix_transform.hpp>

WindowRenderer::WindowRenderer(SDL_Window * window, SDL_GLContext context) : mWindow(window), mContext(context)
{
    int w,h;
    SDL_GetWindowSize(mWindow, &w, &h);

    glViewport(0, 0, w, h);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    Ortho = glm::ortho(0.0f, (float)h, (float)w, 0.0f, -1.0f, 1.0f);
}

WindowRenderer::~WindowRenderer()
{
    SDL_GL_DeleteContext(mContext);
    SDL_DestroyWindow(mWindow);
}

void WindowRenderer::SetBackgroundColour(const glm::vec4 &colour)
{
    SDL_GL_MakeCurrent(mWindow, mContext);
    glClearColor(colour.r, colour.g, colour.b, colour.a);
}

void WindowRenderer::AddDrawable(Renderer::Drawable & drawable)
{
    mDrawables.push_back(&drawable);
}

void WindowRenderer::Render()
{
    if(SDL_GL_MakeCurrent(mWindow, mContext) < 0)
    {
        throw std::runtime_error(SDL_GetError());
    }

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

    for(auto drawable : mDrawables)
    {
        drawable->Render(Ortho);
    }

    CHECK_GL_ERROR_DEBUG();

    SDL_GL_SwapWindow(mWindow);
}
