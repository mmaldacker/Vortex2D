#include "glfw.h"

#include <Vortex2D/Vortex2D.h>
#include <Vortex2D/Renderer/Device.h>
#include <Vortex2D/Renderer/RenderWindow.h>

#include "RenderExample.h"
#include "SmokeExample.h"

#include <iostream>
#include <memory>
#include <functional>
#include <chrono>

using namespace Vortex2D;

glm::vec4 green = glm::vec4(35.0f, 163.0f, 143.0f, 255.0f)/glm::vec4(255.0f);
glm::vec4 gray = glm::vec4(182.0f,172.0f,164.0f, 255.0f)/glm::vec4(255.0f);
glm::vec4 blue = glm::vec4(99.0f, 155.0f, 188.0f, 255.0f)/glm::vec4(255.0f);
glm::vec4 colour = glm::vec4{99.0f,96.0f,93.0f,255.0f} / glm::vec4(255.0f);

glm::ivec2 size = {1000,1000};
float scale = 4;

Vortex2D::Renderer::Device* device;
Vortex2D::Renderer::RenderTarget* target;

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
            example.reset(new SmokeExample(*device, {size, scale}, 0.033f));
            break;
        case GLFW_KEY_3:
            //example.reset(new ObstacleSmokeExample(size, 0.033f));
            break;
        case GLFW_KEY_4:
            //example.reset(new WaterExample(size, 0.033f));
            break;
        case GLFW_KEY_5:
            //example.reset(new ScaleWaterExample(size, 0.033f));
        default:
            break;
    }

    Vortex2D::Renderer::RenderState t(*target);
    t.ColorBlend
            .setBlendEnable(true)
            .setAlphaBlendOp(vk::BlendOp::eAdd)
            .setColorBlendOp(vk::BlendOp::eAdd)
            .setSrcColorBlendFactor(vk::BlendFactor::eSrcAlpha)
            .setSrcAlphaBlendFactor(vk::BlendFactor::eOne)
            .setDstColorBlendFactor(vk::BlendFactor::eOneMinusSrcAlpha)
            .setDstAlphaBlendFactor(vk::BlendFactor::eZero);

    example->Initialize(t);

    target->Record([&](vk::CommandBuffer commandBuffer)
    {
        Renderer::Clear(size.x, size.y, {0.5f, 0.5f, 0.5f, 1.0f}).Draw(commandBuffer);
        example->Draw(commandBuffer, t);
    });
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

        target->Record([&](vk::CommandBuffer commandBuffer)
        {
            Renderer::Clear(size.x, size.y, {0.5f, 0.5f, 0.5f, 1.0f}).Draw(commandBuffer);
        });

        while(!mainWindow.ShoudCloseWindow())
        {
            glfwPollEvents();

            auto start = std::chrono::system_clock::now();

            if (example) example->Update(window.Orth, glm::mat4());
            window.Submit();

            auto end = std::chrono::system_clock::now();
            auto duration = end - start;
            auto title = std::to_string(std::chrono::duration_cast<std::chrono::milliseconds>(duration).count()) + "ms";
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
