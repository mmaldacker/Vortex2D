#include <Vortex2D/Vortex2D.h>
#include <GLFW/glfw3.h>

#include "RenderExample.h"
#include "SmokeExample.h"
#include "ObstacleSmokeExample.h"
#include "WaterExample.h"
#include "SmokeVelocityExample.h"
#include "BuoyancyWaterExample.h"
#include "WaterFallExample.h"
#include "WatermillExample.h"

#include <iostream>
#include <memory>
#include <functional>
#include <chrono>
#include <numeric>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/transform.hpp>

using namespace Vortex2D;

glm::vec4 red = glm::vec4(242.0f, 95.0f, 92.0f, 255.0f) / glm::vec4(255.0f);
glm::vec4 green = glm::vec4(112.0f, 193.0f, 179.0f, 255.0f) / glm::vec4(255.0f);
glm::vec4 gray = glm::vec4(80.0f, 81.0f, 79.0f, 255.0f) / glm::vec4(255.0f);
glm::vec4 blue = glm::vec4(36.0f, 123.0f, 160.0f, 255.0f) / glm::vec4(255.0f);
glm::vec4 yellow = glm::vec4(255.0f, 224.0f, 102.0f, 255.0f) / glm::vec4(255.0f);

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

vk::SurfaceKHR GetGLFWSurface(GLFWwindow* window, vk::Instance instance)
{
    // create surface
    VkSurfaceKHR surface;
    if (glfwCreateWindowSurface(static_cast<VkInstance>(instance), window, nullptr, &surface) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create window surface!");
    }

    return surface;
}

glm::vec2 GetGLFWindowScale(GLFWwindow* window)
{
    glm::vec2 scale;
    glfwGetWindowContentScale(window, &scale.x, &scale.y);

    return scale;
}

GLFWwindow* GetGLFWWindow(const glm::ivec2& size)
{
    GLFWwindow* window = glfwCreateWindow(size.x, size.y, "Vortex2D App", nullptr, nullptr);
    if (window == nullptr)
    {
        throw std::runtime_error("Could not create glfw window");
    }

    glm::vec2 scale = GetGLFWindowScale(window);
    glfwSetWindowSize(window, size.x * scale.x, size.y * scale.y);

    return window;
}

class App
{
public:
    glm::ivec2 size = {256, 256};
    glm::ivec2 windowSize = {1024, 1024};
    float delta = 0.016f;

    App(bool validation = true)
        : glfwWindow(GetGLFWWindow(windowSize))
        , scale(GetGLFWindowScale(glfwWindow))
        , instance("Vortex2D", GetGLFWExtensions(), validation)
        , surface(GetGLFWSurface(glfwWindow, static_cast<VkInstance>(instance.GetInstance())))
        , device(instance.GetPhysicalDevice(), surface, validation)
        , window(device, surface, windowSize.x * scale.x, windowSize.y * scale.y)
        , clearRender(window.Record({clear}))
    {
        // setup callback
        glfwSetWindowUserPointer(glfwWindow, this);
        auto func = [](GLFWwindow* window, int key, int scancode, int action, int mods)
        {
            static_cast<App*>(glfwGetWindowUserPointer(window))->KeyCallback(key, action);
        };
        glfwSetKeyCallback(glfwWindow, func);

        window.View = glm::scale(glm::vec3{4.0f * scale.x, 4.0f * scale.y, 1.0f});
    }

    ~App()
    {
        device.Handle().waitIdle();
        instance.GetInstance().destroySurfaceKHR(surface);
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
            example.reset(new SmokeExample(device, size, delta));
            break;
        case GLFW_KEY_3:
            example.reset(new ObstacleSmokeExample(device, size, delta));
            break;
        case GLFW_KEY_4:
            example.reset(new SmokeVelocityExample(device, size, delta));
            break;
        case GLFW_KEY_5:
            example.reset(new WaterExample(device, size, delta));
            break;
        case GLFW_KEY_6:
            example.reset(new WaterFallExample(device, size, delta));
            break;
        case GLFW_KEY_7:
            example.reset(new WatermillExample(device, size, delta));
            break;
        case GLFW_KEY_8:
            example.reset(new HydrostaticWaterExample(device, size, delta));
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
    glm::vec2 scale;
    Vortex2D::Renderer::Instance instance;
    vk::SurfaceKHR surface;
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

        App app;
        app.Run();
    }
    catch (const std::exception& error)
    {
        std::cout << "exception: " << error.what() << std::endl;
    }

    glfwTerminate();
}
