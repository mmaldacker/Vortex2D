#include "Common.h"
#include "ResourcePath.h"
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

        // -------------------

        Renderer::Rectangle source({60.0f, 60.0f});
        source.Position = {200.0f, 200.0f};
        source.Colour = {-1.0f, -2.0f, 0.0f, 0.0f};

        Renderer::Rectangle blocked({30.0f, 30.0f});
        blocked.Position = {120.0f, 120.0f};
        blocked.Colour = {-1.0f, -2.0f, 0.0f, 0.0f};

        std::vector<Renderer::Drawable*> sources = {&source, &blocked};

        Renderer::Rectangle rect({80.0f, 80.0f});
        rect.Position = {100.0f, 100.0f};
        rect.Colour = {1.0f, 1.0f, 1.0f, 1.0f};

        std::vector<Renderer::Drawable*> borders = {&rect};

        // -------------------

        Renderer::Disable d(GL_BLEND);

        Fluid::Dimensions dimensions(realSize, scale);
        Fluid::Advection advection(dimensions, 0.33);
        Fluid::Boundaries boundaries(dimensions, 2);

        boundaries.Render(advection, borders);
        boundaries.RenderWeights();
        advection.RenderVelocity(sources);

        glFlush();

        auto velocity = advection.GetVelocityReader();
        velocity.Read().Print().PrintStencil();

        //boundaries.GetReader().Read().Print();
        //boundaries.GetWeightsReader().Read().Print();

        advection.Advect();

        velocity.Read().Print();
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
