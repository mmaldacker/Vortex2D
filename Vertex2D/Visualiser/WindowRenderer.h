//
//  WindowRenderer.h
//  Vortex
//
//  Created by Maximilian Maldacker on 08/04/2014.
//
//

#ifndef __Vortex__WindowRenderer__
#define __Vortex__WindowRenderer__

#include <GLFW/glfw3.h>
#include <vector>

#include "Common.h"
#include "Drawable.h"


class WindowRenderer
{
public:
    WindowRenderer(const glm::vec2 & size, WindowRenderer * shared = NULL);
    ~WindowRenderer();

    void SetBackgroundColour(const glm::vec4 & colour);

    void AddDrawable(Renderer::Drawable & drawable);
    void Render();
    void MakeCurrent();

    bool ShouldClose();

    glm::mat4 Ortho;

private:
    GLFWwindow * mWindow;
    glm::vec4 mBackgroundColour;

    std::vector<Renderer::Drawable*> mDrawables;
};


#endif /* defined(__Vortex__WindowRenderer__) */
