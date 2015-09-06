#include "Common.h"
#include "ResourcePath.h"
#include "GridVisualiser.h"
#include "WindowRenderer.h"
#include "Engine.h"

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

        GridVisualiser grid({size, size}, scale);

        auto realSize = glm::vec2{size*scale,size*scale};
        WindowRenderer window(realSize, &grid);

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

        grid.MakeCurrent();

        /*
         for(int i = 0 ; i < dimensions.Size.x ; i++)
         {
         for(int j = 0 ; j < dimensions.Size.y ; j++)
         {
         grid.RenderValue({i,j}, reader.GetVec4(i, j).x);
         }
         }
        */
        /*
         auto v = reader.GetVec4(5,5).x;
         grid.RenderValue({5,5}, v);
         
         while (!window.ShouldClose() && !grid.ShouldClose())
         {
         //window.Render();
         grid.Render();
         
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

    CHECK_GL_ERROR_DEBUG();

    glfwTerminate();
}
