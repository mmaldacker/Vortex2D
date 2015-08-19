//
//  WindowRenderer.h
//  Vortex
//
//  Created by Maximilian Maldacker on 08/04/2014.
//
//

#ifndef __Vortex__WindowRenderer__
#define __Vortex__WindowRenderer__

#include <vector>

#include "Common.h"
#include "Drawable.h"

namespace Renderer
{

class WindowRenderer
{
public:
    WindowRenderer(SDL_Window * window);

    void SetBackgroundColour(const glm::vec4 & colour);

    void AddDrawable(Drawable * drawable);
    void Render();

    glm::mat4 Ortho;

private:
    SDL_Window * mWindow;
    glm::vec4 mBackgroundColour;

    std::vector<Drawable*> mDrawables;
};

}

#endif /* defined(__Vortex__WindowRenderer__) */
