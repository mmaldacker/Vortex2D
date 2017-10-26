#include "glfw.h"

#include <Vortex2D/Vortex2D.h>
#include <Vortex2D/Renderer/Device.h>
#include <Vortex2D/Renderer/RenderWindow.h>

#include "RenderExample.h"
#include "SmokeExample.h"
#include "ObstacleSmokeExample.h"
#include "WaterExample.h"

#include <iostream>
#include <memory>
#include <functional>
#include <chrono>
#include <numeric>

using namespace Vortex2D;

glm::vec4 green = glm::vec4(35.0f, 163.0f, 143.0f, 255.0f)/glm::vec4(255.0f);
glm::vec4 gray = glm::vec4(182.0f,172.0f,164.0f, 255.0f)/glm::vec4(255.0f);
glm::vec4 blue = glm::vec4(188.0f, 155.0f, 99.0f, 255.0f)/glm::vec4(255.0f);
glm::vec4 colour = glm::vec4{99.0f,96.0f,93.0f,255.0f} / glm::vec4(255.0f);

glm::ivec2 size = {1024,1024};
float scale = 4;
float delta = 0.016f;

Vortex2D::Renderer::Device* device;
Vortex2D::Renderer::RenderTarget* target;
Renderer::Clear* clear;

std::unique_ptr<Vortex2D::Renderer::Drawable> example;

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if (action != GLFW_PRESS) return;

    switch (key)
    {
        case GLFW_KEY_1:
            example.reset(new RenderExample(*device, size));
            break;
        case GLFW_KEY_2:
            example.reset(new SmokeExample(*device, {size, scale}, delta));
            break;
        case GLFW_KEY_3:
            example.reset(new ObstacleSmokeExample(*device, {size, scale}, delta));
            break;
        case GLFW_KEY_4:
            example.reset(new WaterExample(*device, {size, scale}, delta));
            break;
        case GLFW_KEY_5:
            //example.reset(new ScaleWaterExample(size, 0.033f));
        default:
            return;
    }

    auto blendMode = vk::PipelineColorBlendAttachmentState()
            .setBlendEnable(true)
            .setAlphaBlendOp(vk::BlendOp::eAdd)
            .setColorBlendOp(vk::BlendOp::eAdd)
            .setSrcColorBlendFactor(vk::BlendFactor::eSrcAlpha)
            .setSrcAlphaBlendFactor(vk::BlendFactor::eOne)
            .setDstColorBlendFactor(vk::BlendFactor::eOneMinusSrcAlpha)
            .setDstAlphaBlendFactor(vk::BlendFactor::eZero);

    target->Record({*clear, *example}, blendMode);
}

int main()
{
    try
    {
        GLFWApp mainWindow(size.x, size.y);

        glfwSetKeyCallback(mainWindow.GetWindow(), key_callback);

        Renderer::Device device_(mainWindow.GetPhysicalDevice(),
                                 mainWindow.GetSurface());
        device = &device_;

        Renderer::RenderWindow window(device_, mainWindow.GetSurface(), size.x, size.y);
        target = &window;

        Renderer::Clear clear_(size.x, size.y, {0.5f, 0.5f, 0.5f, 1.0f});
        clear = &clear_;

        target->Record({*clear});

        int windowSize = 20;
        std::vector<uint64_t> timePoints(windowSize, 0);
        int timePointIndex = 0;

        while(!mainWindow.ShoudCloseWindow())
        {
            glfwPollEvents();

            auto start = std::chrono::system_clock::now();

            if (example) example->Update(window.Orth, glm::mat4());
            window.Submit();

            auto end = std::chrono::system_clock::now();
            timePoints[timePointIndex] = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
            timePointIndex = (timePointIndex + 1) % windowSize;

            auto average = std::accumulate(timePoints.begin(), timePoints.end(), 0) / windowSize;
            auto title = std::to_string(average) + "ms";
            glfwSetWindowTitle(mainWindow.GetWindow(), title.c_str());
        }

        device_.Handle().waitIdle();

        example.reset();
    }
    catch (const std::exception& error)
    {
        std::cout << "exception: " << error.what() << std::endl;
    }
}
