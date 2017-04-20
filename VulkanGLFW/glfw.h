//
//  GLFW.h
//  Vortex2D
//

#ifndef GLFW_H
#define GLFW_H

#include <GLFW/glfw3.h>

class GLFWApp
{
public:
    GLFWApp(int width, int height, bool visible = true);
    ~GLFWApp();

    void SetKeyCallback(GLFWkeyfun cbfun);

    void Run();

private:
    GLFWwindow * mWindow;
};

#endif
