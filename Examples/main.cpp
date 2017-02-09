#include "RenderWindow.h"
#include "GLFW.h"

#include <Vortex2D/Engine/LineIntegralConvolution.h>

#include "SmokeExample.h"
#include "ObstacleSmokeExample.h"
#include "WaterExample.h"
#include "VelocitySmokeExample.h"
#include "ScaleWaterExample.h"

#include <iostream>
#include <memory>

std::unique_ptr<BaseExample> example;

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if (action != GLFW_PRESS) return;

    switch (key)
    {
        case GLFW_KEY_1:
            example.reset(new SmokeExample());
            break;
        case GLFW_KEY_2:
            example.reset(new ObstacleSmokeExample());
            break;
        case GLFW_KEY_3:
            example.reset(new VelocitySmokeExample());
            break;
        case GLFW_KEY_4:
            example.reset(new WaterExample());
            break;
        case GLFW_KEY_5:
            example.reset(new ScaleWaterExample());
            break;
        default:
            break;
    }

}

int main(int argc, const char * argv[])
{
    GLFW glfw;

    auto colour = glm::vec4{99.0f,96.0f,93.0f,255.0f} / glm::vec4(255.0f);
    glm::vec2 size = {500,500};

    RenderWindow mainWindow(size.x, size.y, "Vortex2D Examples");
    mainWindow.SetKeyCallback(key_callback);
    example.reset(new SmokeExample());

    RenderWindow debugWindow(size.x, size.y, "Debug Window", &mainWindow);
    Vortex2D::Fluid::LineIntegralConvolution lic(size);

    while (!mainWindow.ShouldClose() && !debugWindow.ShouldClose())
    {
        glfwPollEvents();

        mainWindow.MakeCurrent();
        example->Frame();
        mainWindow.Clear(colour);
        example->Render(mainWindow);
        mainWindow.Display();

        debugWindow.MakeCurrent();
        debugWindow.Clear(colour);
        lic.Render(debugWindow);
        debugWindow.Display();
    }
}
