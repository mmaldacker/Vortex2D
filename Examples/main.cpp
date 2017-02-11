#include "RenderWindow.h"
#include "glfw.h"

#include <Vortex2D/Vortex2D.h>
#include <Vortex2D/Engine/LineIntegralConvolution.h>

#include <iostream>
#include <memory>
#include <functional>

glm::vec4 green = glm::vec4(35.0f, 163.0f, 143.0f, 255.0f)/glm::vec4(255.0f);
glm::vec4 gray = glm::vec4(182.0f,172.0f,164.0f, 255.0f)/glm::vec4(255.0f);
glm::vec4 blue = glm::vec4(99.0f, 155.0f, 188.0f, 255.0f)/glm::vec4(255.0f);

using Factory = std::function<std::unique_ptr<Vortex2D::Renderer::Drawable>(Vortex2D::Fluid::Dimensions dimensions, float dt)>;
std::vector<Factory> examplesFactories;

std::size_t currentExample = 0;
std::vector<std::unique_ptr<Vortex2D::Renderer::Drawable>> examples;

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if (action != GLFW_PRESS) return;

    switch (key)
    {
        case GLFW_KEY_SPACE:
            currentExample = (currentExample + 1) % examples.size();
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
    Vortex2D::Fluid::Dimensions dimensions(size, 2.0f);

    RenderWindow mainWindow(size.x, size.y, "Vortex2D Examples");
    mainWindow.SetKeyCallback(key_callback);

    // FIXME maybe don't instantiate all examples at once to reduce memory usage?
    for (auto&& factory : examplesFactories)
    {
        examples.push_back(factory(dimensions, 0.033f));
    }

    RenderWindow debugWindow(size.x, size.y, "Debug Window", &mainWindow);
    Vortex2D::Fluid::LineIntegralConvolution lic(size);

    while (!mainWindow.ShouldClose() && !debugWindow.ShouldClose())
    {
        glfwPollEvents();

        mainWindow.MakeCurrent();
        mainWindow.Clear(colour);
        examples[currentExample]->Render(mainWindow);
        mainWindow.Display();

        debugWindow.MakeCurrent();
        debugWindow.Clear(colour);
        lic.Render(debugWindow);
        debugWindow.Display();
    }
}
