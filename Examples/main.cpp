#include "glfw.h"

#include <Vortex2D/Vortex2D.h>
#include <Vortex2D/Renderer/Device.h>
#include <Vortex2D/Renderer/RenderWindow.h>

#include "RenderExample.h"

#include <iostream>
#include <memory>
#include <functional>

using namespace Vortex2D;

glm::vec4 green = glm::vec4(35.0f, 163.0f, 143.0f, 255.0f)/glm::vec4(255.0f);
glm::vec4 gray = glm::vec4(182.0f,172.0f,164.0f, 255.0f)/glm::vec4(255.0f);
glm::vec4 blue = glm::vec4(99.0f, 155.0f, 188.0f, 255.0f)/glm::vec4(255.0f);

glm::vec2 size = {1000,1000};

std::unique_ptr<Vortex2D::Renderer::Drawable> example;

/*
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if (action != GLFW_PRESS) return;

    switch (key)
    {
        case GLFW_KEY_1:
            example.reset(new SmokeExample(size, 0.033f));
            break;
        case GLFW_KEY_2:
            example.reset(new ObstacleSmokeExample(size, 0.033f));
            break;
        case GLFW_KEY_3:
            example.reset(new WaterExample(size, 0.033f));
            break;
        case GLFW_KEY_4:
            example.reset(new ScaleWaterExample(size, 0.033f));
        default:
            break;
    }

}
*/

int main()
{
    try
    {
        auto colour = glm::vec4{99.0f,96.0f,93.0f,255.0f} / glm::vec4(255.0f);

        GLFWApp mainWindow(size.x, size.y);

        Renderer::Device device(mainWindow.GetPhysicalDevice(),
                                mainWindow.GetSurface());

        Renderer::RenderWindow window(device, mainWindow.GetSurface(), size.x, size.y);

        example.reset(new RenderExample(device, size));

        example->Initialize({window});

        example->Update(window.Orth, glm::mat4());

        window.Record([&](vk::CommandBuffer commandBuffer)
        {
            Renderer::Clear(size.x, size.y, {0.5f, 0.5f, 0.5f, 1.0f}).Draw(commandBuffer);
            example->Draw(commandBuffer, {window});
        });

        while(!mainWindow.ShoudCloseWindow())
        {
            glfwPollEvents();
            window.Submit();
        }

        device.Handle().waitIdle();

        example.reset();
    }
    catch (const std::exception& error)
    {
        std::cout << "exception: " << error.what() << std::endl;
    }
}
