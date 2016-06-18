#include "Common.h"
#include "RenderWindow.h"
#include "SmokeExample.h"
#include "ObstacleSmokeExample.h"
#include "WaterExample.h"
#include "VelocitySmokeExample.h"
#include "ScaleWaterExample.h"

#include <iostream>
#include <GLFW/glfw3.h>

std::unique_ptr<BaseExample> example;

void error_callback(int error, const char* description)
{
    std::cout << "GLFW erro " << error << " : " << description << std::endl;
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if(action != GLFW_PRESS) return;

    switch(key)
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
    if(!glfwInit()) { std::cout << "Could not initialize GLFW" << std::endl; exit(EXIT_FAILURE); }

    glfwSetErrorCallback(error_callback);

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    glm::vec2 size = {500,500};

    GLFWwindow * window = glfwCreateWindow(size.x, size.y, "Vortex2D Examples", NULL, NULL);

    glfwMakeContextCurrent(window);

    glewExperimental = GL_TRUE; //stops glew crashing on OSX :-/
    if(glewInit() != GLEW_OK)
        throw std::runtime_error("glewInit failed");

    glfwSetKeyCallback(window, key_callback);

    glViewport(0, 0, size.x, size.y);

    glfwSwapInterval(1);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    auto colour = glm::vec4{99.0f,96.0f,93.0f,255.0f}/glm::vec4(255.0f);

    glDisable(GL_DEPTH_TEST);

    example.reset(new SmokeExample());

    RenderWindow renderWindow(size.x, size.y);

    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();

        example->Frame();

        renderWindow.Clear(colour);
        example->Render(renderWindow);

        glfwSwapBuffers(window);
    }

    glfwDestroyWindow(window);

    glfwTerminate();
}
