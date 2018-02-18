#include "glfw.h"

#include <Vortex2D/Vortex2D.h>
#include <Vortex2D/Renderer/Device.h>
#include <Vortex2D/Renderer/RenderWindow.h>

#include "RenderExample.h"
#include "SmokeExample.h"
#include "ObstacleSmokeExample.h"
#include "WaterExample.h"
#include "SmokeVelocityExample.h"

#include <iostream>
#include <memory>
#include <functional>
#include <chrono>
#include <numeric>

using namespace Vortex2D;

glm::vec4 green = glm::vec4(35.0f, 163.0f, 143.0f, 255.0f)/glm::vec4(255.0f);
glm::vec4 gray = glm::vec4(182.0f,172.0f,164.0f, 255.0f)/glm::vec4(255.0f);
glm::vec4 blue = glm::vec4(188.0f, 155.0f, 99.0f, 255.0f)/glm::vec4(255.0f);

class App
{
public:
    glm::ivec2 size = {1024,1024};
    float scale = 4;
    float delta = 0.016f;

    App()
        : mainWindow(size.x, size.y)
        , device(mainWindow.GetPhysicalDevice(), mainWindow.GetSurface())
        , window(device, mainWindow.GetSurface(), size.x, size.y)
        , clearRender(window.Record({clear}))
    {
        glfwSetWindowUserPointer(mainWindow.GetWindow(), this);

        auto func = [](GLFWwindow* window, int key, int scancode, int action, int mods)
        {
            static_cast<App*>(glfwGetWindowUserPointer(window))->KeyCallback(key, action);
        };

        glfwSetKeyCallback(mainWindow.GetWindow(), func);
    }

    ~App()
    {
        device.Handle().waitIdle();
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
        default:
            return;
        }

        example->Init(device, window);
    }

    void Run()
    {
        const int windowSize = 20;
        std::vector<uint64_t> timePoints(windowSize, 0);
        int timePointIndex = 0;

        while(!mainWindow.ShoudCloseWindow())
        {
            glfwPollEvents();

            auto start = std::chrono::system_clock::now();

            clearRender.Submit();
            if (example) example->Step();
            window.Display();

            auto end = std::chrono::system_clock::now();
            timePoints[timePointIndex] = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
            timePointIndex = (timePointIndex + 1) % windowSize;

            auto average = std::accumulate(timePoints.begin(), timePoints.end(), 0) / windowSize;
            auto title = std::to_string(average) + "ms";
            glfwSetWindowTitle(mainWindow.GetWindow(), title.c_str());
        }
    }

    GLFWApp mainWindow;
    Vortex2D::Renderer::Device device;
    Renderer::RenderWindow window;
    Renderer::Clear clear = {{0.5f, 0.5f, 0.5f, 1.0f}};
    std::unique_ptr<Runner> example;
    Vortex2D::Renderer::RenderCommand clearRender;
};

int main()
{
    try
    {
        App app;
        app.Run();
    }
    catch (const std::exception& error)
    {
        std::cout << "exception: " << error.what() << std::endl;
    }
}
