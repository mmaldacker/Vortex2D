//
//  WindowRenderer.h
//  Vortex
//
//  Created by Maximilian Maldacker on 08/04/2014.
//
//

#ifndef __Vortex__WindowRenderer__
#define __Vortex__WindowRenderer__

#include <SDL2/SDL.h>
#include <vector>

#include "Common.h"
#include "Drawable.h"


class WindowRenderer
{
public:
    WindowRenderer(SDL_Window * window, SDL_GLContext context);
    ~WindowRenderer();

    void SetBackgroundColour(const glm::vec4 & colour);

    void AddDrawable(Renderer::Drawable & drawable);
    void Render();

    glm::mat4 Ortho;

private:
    SDL_Window * mWindow;
    SDL_GLContext mContext;
    glm::vec4 mBackgroundColour;

    std::vector<Renderer::Drawable*> mDrawables;
};


#endif /* defined(__Vortex__WindowRenderer__) */
