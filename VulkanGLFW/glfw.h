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
    Vortex2D::Renderer::Device GetDevice();

    void Run();

private:
    uint32_t mWidth, mHeight;
    bool mValidation;
    GLFWwindow * mWindow;
    vk::UniqueInstance mInstance;
    vk::UniqueDebugReportCallbackEXT mDebugCallback;
    vk::UniqueSurfaceKHR mSurface;
    vk::UniqueDevice mDevice;
    vk::Queue mQueue;
    vk::UniqueSwapchainKHR mSwapChain;
    int GetFamilyIndex(vk::PhysicalDevice physicalDevice);
};

#endif
