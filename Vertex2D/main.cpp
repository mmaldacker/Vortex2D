#include "Common.h"
#include "ResourcePath.h"
#include "WindowRenderer.h"
#include "Engine.h"
#include "SuccessiveOverRelaxation.h"
#include "Disable.h"
#include "Multigrid.h"
#include "Reduce.h"
#include "Sprite.h"

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
        glm::vec2 size = glm::vec2(10);
        int scale = 60;

        auto realSize = size * glm::vec2{scale};
        WindowRenderer window(realSize);
        window.SetBackgroundColour(glm::vec4{99.0f,96.0f,93.0f,255.0f}/glm::vec4(255.0f));

        // -------------------

        Renderer::Rectangle source({60.0f, 60.0f});
        source.Position = {400.0f, 400.0f};
        source.Colour = {-80.0f, -100.0f, 0.0f, 0.0f};

        std::vector<Renderer::Drawable*> sources = {&source};

        Renderer::Rectangle rect({90.0f, 90.0f});
        rect.Position = {120.0f, 120.0f};
        rect.Colour = {1.0f, 1.0f, 1.0f, 1.0f};

        std::vector<Renderer::Drawable*> borders;// = {&rect};

        Renderer::Rectangle density({60.0f, 60.0f});
        density.Position = (glm::vec2)source.Position;
        density.Colour = glm::vec4{182.0f,172.0f,164.0f, 255.0f}/glm::vec4(255.0f);

        std::vector<Renderer::Drawable*> densities = {&density};

        // -------------------

        Renderer::Disable d(GL_BLEND);
        glDisable(GL_DEPTH_TEST);

        Fluid::Dimensions dimensions(realSize, scale);
        Fluid::Advection advection(dimensions, 0.033);
        Fluid::Boundaries boundaries(dimensions, 2);

        Fluid::SuccessiveOverRelaxation sor(size);
        Fluid::Multigrid multigrid(size);

        Fluid::Engine engine(dimensions, boundaries, advection, &sor);

        Renderer::Sprite sprite{advection.mDensity.Front};

        boundaries.Render(borders);
        //boundaries.GetReader().Read().Print();

        advection.RenderMask(boundaries);
        advection.RenderVelocity(sources);

        engine.LinearInit(boundaries);

        engine.Div();
/*
        multigrid.GetWeightsReader(0).Read().Print();
        multigrid.GetWeightsReader(1).Read().Print();

        multigrid.GetPressureReader(0).Read().Print();
        multigrid.GaussSeidel(0, true);
        multigrid.GetPressureReader(0).Read().Print();
        multigrid.Residual(0);
        multigrid.GetPressureReader(0).Read().Print();
        multigrid.Restrict(0);
        multigrid.GetPressureReader(1).Read().Print();
        multigrid.GaussSeidel(1, true);
        multigrid.GetPressureReader(1).Read().Print();
        multigrid.Prolongate(0);
        multigrid.GetPressureReader(0).Read().Print();
        multigrid.Correct(0);
        multigrid.GetPressureReader(0).Read().Print();
        multigrid.GaussSeidel(0, false);
        multigrid.GetPressureReader(0).Read().Print();
*/

        Fluid::Reduce reduce(size);

        reduce.apply(boundaries.mBoundaries, boundaries.mBoundaries);
        std::cout << reduce.Get() << std::endl;

        //engine.LinearSolve();

        //Renderer::Reader{sor.GetData().Pressure.Front}.Read().Print();
        //Renderer::Reader{multigrid.mXs[0].mX.Front}.Read().Print();
/*
        while (!window.ShouldClose())
        {
            glfwPollEvents();

            advection.RenderVelocity(sources);
            advection.RenderDensity(densities);

            engine.Div();

            engine.LinearSolve();

            engine.Project();

            advection.Advect();

            Renderer::Enable d(GL_BLEND);
            window.Clear();
            window.Render({&sprite});
            window.Swap();
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
