//
//  WindowRenderer.cpp
//  Vortex
//
//  Created by Maximilian Maldacker on 08/04/2014.
//
//

#include "WindowRenderer.h"

#include <glm/gtc/matrix_transform.hpp>

WindowRenderer::WindowRenderer(const glm::vec2 & size)
{
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

    mWindow = SDL_CreateWindow(NULL, 0, 0, size.x, size.y, SDL_WINDOW_OPENGL);
    mContext = SDL_GL_CreateContext(mWindow);
    if(!mContext)
    {
        throw std::runtime_error(SDL_GetError());
    }

    glViewport(0, 0, size.x, size.y);

    Ortho = glm::ortho(0.0f, size.x, size.y, 0.0f, -1.0f, 1.0f);
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

void WindowRenderer::AddDrawable(Renderer::Drawable * drawable)
{
    mDrawables.push_back(drawable);
}

void WindowRenderer::Render()
{
    if(SDL_GL_MakeCurrent(mWindow, mContext))
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
