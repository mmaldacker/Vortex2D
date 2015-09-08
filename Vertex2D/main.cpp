#include "Common.h"
#include "ResourcePath.h"
#include "GridVisualiser.h"
#include "WindowRenderer.h"
#include "Engine.h"
#include "Disable.h"

#include <string>

void error_callback(int error, const char* description)
{
    std::cout << "GLFW erro " << error << " : " << description << std::endl;
}

struct Main
{
    Main()
    {
        int size = 12;
        int scale = 30;

        auto realSize = glm::vec2{size*scale,size*scale};
        WindowRenderer window(realSize);

        Renderer::Disable d(GL_BLEND);

        Fluid::Dimensions dimensions(realSize, scale);
        Fluid::Boundaries boundaries(dimensions, 2);

        Renderer::Rectangle rect({80.0f, 80.0f});
        rect.Position = {100.0f, 100.0f};
        rect.Colour = {1.0f, 1.0f, 1.0f, 1.0f};
        boundaries.Render({&rect});
        boundaries.RenderWeights();

        glFlush();

        auto reader = boundaries.GetReader();
        reader.Read();

        auto weights = boundaries.GetWeightsReader();
        weights.Read();

        Renderer::Rectangle source({60.0f, 60.0f});
        source.Position = {240.0f, 240.0f};
        source.Colour = {34.5f, 12.0f, 0.0f, 0.0f};

        Fluid::Advection advection(dimensions, boundaries, 0.33);

        advection.RenderVelocity({&source});

        auto velocity = advection.GetVelocityReader();
        velocity.Read();

        /*
         auto v = reader.GetVec4(5,5).x;
         grid.RenderValue({5,5}, v);
         
         while (!window.ShouldClose() && !grid.ShouldClose())
         {
         window.Render();

         glfwPollEvents();
         }
        */
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
