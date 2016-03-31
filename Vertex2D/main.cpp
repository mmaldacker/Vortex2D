#include "Common.h"
#include "ResourcePath.h"
#include "WindowRenderer.h"
#include "Engine.h"
#include "SuccessiveOverRelaxation.h"
#include "Disable.h"
#include "Reduce.h"
#include "Sprite.h"
#include "ConjugateGradient.h"

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
        glm::vec2 size = glm::vec2(20);
        int scale = 1;

        auto realSize = size * glm::vec2{scale};
        WindowRenderer window(realSize);
        window.SetBackgroundColour(glm::vec4{99.0f,96.0f,93.0f,255.0f}/glm::vec4(255.0f));

        // -------------------

        Renderer::Rectangle rect1({6.0f, 6.0f});
        rect1.Position = {4.0f, 4.0f};
        rect1.Colour = {1.0f, 1.0f, 1.0f, 1.0f};

        std::vector<Renderer::Drawable*> dirichlets = {&rect1};

        Renderer::Rectangle rect2({6.0f, 6.0f});
        rect2.Position = {12.0f, 12.0f};
        rect2.Colour = {1.0f, 1.0f, 1.0f, 1.0f};

        std::vector<Renderer::Drawable*> neumanns = {&rect2};

        // -------------------

        Renderer::Disable d(GL_BLEND);
        glDisable(GL_DEPTH_TEST);

        Fluid::Dimensions dimensions(realSize, scale);
        Fluid::Advection advection(dimensions, 0.033);
        Fluid::Boundaries boundaries(dimensions, 2);

        Fluid::SuccessiveOverRelaxation sor(size);
        Fluid::ConjugateGradient cg(size);

        Fluid::Engine engine(dimensions, boundaries, advection, &sor);

        Renderer::Sprite sprite{advection.mDensity.texture()};

        boundaries.RenderNeumann(neumanns);
        boundaries.RenderBorders();
        boundaries.RenderDirichlet(dirichlets);

        boundaries.mDirichletBoundaries.get().Read().Print();
        boundaries.mNeumannBoundaries.get().Read().Print();

        Fluid::LinearSolver::Data data(size);

        data.Weights = boundaries.GetWeights();
        data.Weights.get().Read().Print();

        data.Diagonal = boundaries.GetDiagonals();
        data.Diagonal.get().Read().Print();

        boundaries.RenderMask(data.Pressure);
        data.Pressure.get().Read().PrintStencil();


/*
        advection.RenderMask(boundaries);
        advection.RenderVelocity(sources);

        engine.LinearInit(boundaries);

        engine.Div();

        //engine.mData.Pressure.get().Read().Print();

        engine.LinearSolve();

        //engine.mData.Pressure.get().Read().Print();

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
