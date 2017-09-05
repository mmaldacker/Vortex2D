//
//  GLFW.h
//  Vortex2D
//

#ifndef GLFW_H
#define GLFW_H

#include <vulkan/vulkan.hpp>
#include <GLFW/glfw3.h>

#include <Vortex2D/Renderer/Instance.h>
#include <Vortex2D/Renderer/Device.h>

class GLFWApp
{
public:
    GLFWApp(uint32_t width, uint32_t height, bool visible = true, bool validation = true);

    ~GLFWApp();

    vk::PhysicalDevice GetPhysicalDevice() const;

    // TODO can return a struct with surface, width and height
    vk::SurfaceKHR GetSurface() const;

    vk::Instance GetInstance() const;

    bool ShoudCloseWindow() const;

    GLFWwindow* GetWindow();

private:
    uint32_t mWidth, mHeight;
    GLFWwindow* mWindow;
    Vortex2D::Renderer::Instance mInstance;
    vk::UniqueSurfaceKHR mSurface;
};

#endif
