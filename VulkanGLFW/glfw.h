//
//  GLFW.h
//  Vortex2D
//

#ifndef GLFW_H
#define GLFW_H

#include <vulkan/vulkan.hpp>
#include <GLFW/glfw3.h>

#include <Vortex2D/Renderer/Device.h>

class GLFWApp
{
public:
    GLFWApp(uint32_t width, uint32_t height, bool visible = true, bool validation = true);
    ~GLFWApp();

    void SetKeyCallback(GLFWkeyfun cbfun);
    vk::PhysicalDevice GetPhysicalDevice() const;

    // TODO can return a struct with surface, width and height
    vk::SurfaceKHR GetSurface() const;

    void Run();

private:
    uint32_t mWidth, mHeight;
    GLFWwindow* mWindow;
    vk::UniqueInstance mInstance;
    vk::UniqueDebugReportCallbackEXT mDebugCallback;
    vk::UniqueSurfaceKHR mSurface;
};

#endif
