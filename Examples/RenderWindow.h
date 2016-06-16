//
//  RenderWindow.h
//  Vortex2D
//
//  Created by Maximilian Maldacker on 16/06/2016.
//
//

#ifndef RenderWindow_h
#define RenderWindow_h

#include "RenderTarget.h"

struct RenderWindow : Renderer::RenderTarget
{
    RenderWindow(int width, int height) : Renderer::RenderTarget(width, height)
    {
    }

    void Clear(const glm::vec4 & colour)
    {
        glClearColor(colour.r, colour.g, colour.b, colour.a);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
    }

    void Render(Renderer::Drawable & object, const glm::mat4 & transform)
    {
        object.Render(*this, glm::mat4());
    }
};

#endif /* RenderWindow_h */
