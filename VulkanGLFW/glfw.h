//
//  GLFW.h
//  Vortex2D
//

#ifndef GLFW_H
#define GLFW_H

#include <vulkan/vulkan.hpp>
#include <GLFW/glfw3.h>

class GLFWApp
{
public:
    GLFWApp(int width, int height, bool visible = true, bool validation = true);
    ~GLFWApp();

    void SetKeyCallback(GLFWkeyfun cbfun);

    void Run();

private:
    GLFWwindow * mWindow;
    vk::UniqueInstance mInstance;
    vk::UniqueDebugReportCallbackEXT mDebugCallback;
};

#endif
