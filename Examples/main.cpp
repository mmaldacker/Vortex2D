#include <Vortex2D/Vortex2D.h>
#include <GLFW/glfw3.h>

#include "RenderExample.h"
#include "SmokeExample.h"
#include "ObstacleSmokeExample.h"
#include "WaterExample.h"
#include "SmokeVelocityExample.h"
#include "BallWaterExample.h"

#include <iostream>
#include <memory>
#include <functional>
#include <chrono>
#include <numeric>

using namespace Vortex2D;

glm::vec4 green = glm::vec4(35.0f, 163.0f, 143.0f, 255.0f)/glm::vec4(255.0f);
glm::vec4 gray = glm::vec4(182.0f,172.0f,164.0f, 255.0f)/glm::vec4(255.0f);
glm::vec4 blue = glm::vec4(188.0f, 155.0f, 99.0f, 255.0f)/glm::vec4(255.0f);

static void ErrorCallback(int error, const char* description)
{
    throw std::runtime_error("GLFW Error: " +
                             std::to_string(error) +
                             " What: " +
                             std::string(description));
}

std::vector<const char*> GetGLFWExtensions()
{
    std::vector<const char*> extensions;
    unsigned int glfwExtensionCount = 0;
    const char** glfwExtensions;

    // get the required extensions from GLFW
    glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
    for (unsigned int i = 0; i < glfwExtensionCount; i++)
    {
        extensions.push_back(glfwExtensions[i]);
    }

    return extensions;
}

vk::UniqueSurfaceKHR GetGLFWSurface(GLFWwindow* window, vk::Instance instance)
{
    // create surface
    VkSurfaceKHR surface;
    if (glfwCreateWindowSurface(static_cast<VkInstance>(instance), window, nullptr, &surface) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create window surface!");
    }

    return vk::UniqueSurfaceKHR(surface, vk::SurfaceKHRDeleter{instance});
}

GLFWwindow* GetGLFWWindow(const glm::ivec2& size)
{
    GLFWwindow* window = glfwCreateWindow(size.x, size.y, "Vortex2D App", nullptr, nullptr);
    if (window == nullptr)
    {
        throw std::runtime_error("Could not create glfw window");
    }

    return window;
}

class App
{
public:
    glm::ivec2 size = {1024,1024};
    float scale = 4;
    float delta = 0.016f;

    App(bool validation = true)
        : glfwWindow(GetGLFWWindow(size))
        , instance("Vortex2D", GetGLFWExtensions(), validation)
        , surface(GetGLFWSurface(glfwWindow, static_cast<VkInstance>(instance.GetInstance())))
        , device(instance.GetPhysicalDevice(), *surface, validation)
        , window(device, *surface, size.x, size.y)
        , clearRender(window.Record({clear}))
    {
        // setup callback
        glfwSetWindowUserPointer(glfwWindow, this);
        auto func = [](GLFWwindow* window, int key, int scancode, int action, int mods)
        {
            static_cast<App*>(glfwGetWindowUserPointer(window))->KeyCallback(key, action);
        };
        glfwSetKeyCallback(glfwWindow, func);
    }

    ~App()
    {
        device.Handle().waitIdle();
        glfwDestroyWindow(glfwWindow);
    }

    void KeyCallback(int key, int action)
    {
        if (action != GLFW_PRESS) return;

        device.Handle().waitIdle();

        switch (key)
        {
        case GLFW_KEY_1:
            example.reset(new RenderExample(device, size));
            break;
        case GLFW_KEY_2:
            example.reset(new SmokeExample(device, {size, scale}, delta));
            break;
        case GLFW_KEY_3:
            example.reset(new ObstacleSmokeExample(device, {size, scale}, delta));
            break;
        case GLFW_KEY_4:
            example.reset(new WaterExample(device, {size, scale}, delta));
            break;
        case GLFW_KEY_5:
            example.reset(new SmokeVelocityExample(device, {size, scale}, delta));
            break;
        case GLFW_KEY_6:
            example.reset(new BallWaterExample(device, {size, scale}, delta));
            break;
        default:
            return;
        }

        example->Init(device, window);
    }

    void Run()
    {
        const uint64_t windowSize = 20;
        std::vector<uint64_t> timePoints(windowSize, 0);
        int timePointIndex = 0;

        while(!glfwWindowShouldClose(glfwWindow))
        {
            glfwPollEvents();

            auto start = std::chrono::system_clock::now();

            clearRender.Submit();
            if (example) example->Step();
            window.Display();

            auto end = std::chrono::system_clock::now();
            timePoints[timePointIndex] = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
            timePointIndex = (timePointIndex + 1) % windowSize;

            auto average = std::accumulate(timePoints.begin(), timePoints.end(), 0ll) / windowSize;
            auto title = std::to_string(average) + "ms";
            glfwSetWindowTitle(glfwWindow, title.c_str());
        }
    }

    GLFWwindow* glfwWindow;
    Vortex2D::Renderer::Instance instance;
    vk::UniqueSurfaceKHR surface;
    Vortex2D::Renderer::Device device;
    Renderer::RenderWindow window;
    Renderer::Clear clear = {{0.5f, 0.5f, 0.5f, 1.0f}};
    std::unique_ptr<Runner> example;
    Vortex2D::Renderer::RenderCommand clearRender;
};

int main()
{
    if (!glfwInit())
    {
        throw std::runtime_error("Could not initialise GLFW!");
    }

    try
    {
        glfwSetErrorCallback(ErrorCallback);
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);

        // load symbols
        if (!vkLoaderInit()) throw std::runtime_error("cannot load vulkan library!");

        App app;
        app.Run();
    }
    catch (const std::exception& error)
    {
        std::cout << "exception: " << error.what() << std::endl;
    }

    glfwTerminate();
}
