#include "Common.h"
#include "ResourcePath.h"
#include "WindowRenderer.h"
#include "Engine.h"
#include "SuccessiveOverRelaxation.h"
#include "Disable.h"

#include <string>
#include <chrono>
#include <thread>

void error_callback(int error, const char* description)
{
    std::cout << "GLFW erro " << error << " : " << description << std::endl;
}

struct Main
{
    Main()
    {
        int size = 500;
        int scale = 1;

        auto realSize = glm::vec2{size*scale,size*scale};
        WindowRenderer window(realSize);
        window.SetBackgroundColour({0.0f, 0.0f, 0.0f, 1.0f});

        // -------------------

        Renderer::Rectangle source({60.0f, 60.0f});
        source.Position = {400.0f, 400.0f};
        source.Colour = {-80.0f, -100.0f, 0.0f, 0.0f};

        std::vector<Renderer::Drawable*> sources = {&source};

        Renderer::Rectangle rect({90.0f, 90.0f});
        rect.Position = {120.0f, 120.0f};
        rect.Colour = {1.0f, 1.0f, 1.0f, 1.0f};

        std::vector<Renderer::Drawable*> borders = {&rect};

        Renderer::Rectangle density({60.0f, 60.0f});
        density.Position = (glm::vec2)source.Position;
        density.Colour = {0.0f, 0.0f, 1.0f, 1.0f};

        std::vector<Renderer::Drawable*> densities = {&density};

        // -------------------

        Renderer::Disable d(GL_BLEND);
        glDisable(GL_DEPTH_TEST);

        Fluid::Dimensions dimensions(realSize, scale);
        Fluid::Advection advection(dimensions, 0.033);
        Fluid::Boundaries boundaries(dimensions, 2);
        Fluid::Engine engine(dimensions, boundaries, advection);

        auto & sprite = advection.GetDensity();

        boundaries.Render(borders);
        boundaries.RenderWeights();

        advection.RenderMask(boundaries, borders);
        engine.LinearInit(borders);

        while (!window.ShouldClose())
        {
            glfwPollEvents();

            advection.RenderVelocity(sources);
            advection.RenderDensity(densities);

            engine.Div();

            engine.LinearSolve();

            engine.Project();

            advection.Advect();

            window.Clear();
            window.Render({&sprite});
            window.Swap();
        }
    }
};

int main(int argc, const char * argv[])
{
    if(!glfwInit()) { std::cout << "Could not initialize GLFW" << std::endl; exit(EXIT_FAILURE); }

    glfwSetErrorCallback(error_callback);

    std::cout << "My resource path is " << getResourcePath() << std::endl;

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    Main();

    glfwTerminate();
}
