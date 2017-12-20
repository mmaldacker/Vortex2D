//
//  GLFW.cpp
//  Vortex2D
//

#include "glfw.h"

#include <stdexcept>
#include <string>
#include <iostream>

static void ErrorCallback(int error, const char* description)
{
    throw std::runtime_error("GLFW Error: " +
                             std::to_string(error) +
                             " What: " +
                             std::string(description));
}

GLFWApp::GLFWApp(uint32_t width, uint32_t height, bool validation)
    : mWidth(width)
    , mHeight(height)
{
    glfwSetErrorCallback(ErrorCallback);

    if (!glfwInit())
    {
        throw std::runtime_error("Could not initialise GLFW!");
    }

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);

    mWindow = glfwCreateWindow(width, height, "Vortex2D App", nullptr, nullptr);
    if (!mWindow)
    {
        throw std::runtime_error("Error creating GLFW Window");
    }

    // load symbols
    if (!vkLoaderInit()) throw std::runtime_error("cannot load vulkan library!");

    std::vector<const char*> extensions;
    unsigned int glfwExtensionCount = 0;
    const char** glfwExtensions;

    // get the required extensions from GLFW
    glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
    for (int i = 0; i < glfwExtensionCount; i++)
    {
        extensions.push_back(glfwExtensions[i]);
    }

    mInstance.Create("Vortex2D", extensions, validation);

    // create surface
    VkSurfaceKHR surface;
    if (glfwCreateWindowSurface(static_cast<VkInstance>(mInstance.GetInstance()), mWindow, nullptr, &surface) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create window surface!");
    }

    mSurface = vk::UniqueSurfaceKHR(surface, vk::SurfaceKHRDeleter{mInstance.GetInstance()});
}

GLFWApp::~GLFWApp()
{
    glfwDestroyWindow(mWindow);
    glfwTerminate();
}

vk::SurfaceKHR GLFWApp::GetSurface() const
{
    return *mSurface;
}

vk::Instance GLFWApp::GetInstance() const
{
    return mInstance.GetInstance();
}

vk::PhysicalDevice GLFWApp::GetPhysicalDevice() const
{
    return mInstance.GetPhysicalDevice();
}

bool GLFWApp::ShoudCloseWindow() const
{
    return glfwWindowShouldClose(mWindow);
}

GLFWwindow* GLFWApp::GetWindow()
{
    return mWindow;
}

