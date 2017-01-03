#include "Common.h"
#include "RenderWindow.h"
#include "SmokeExample.h"
#include "ObstacleSmokeExample.h"
#include "WaterExample.h"
#include "VelocitySmokeExample.h"
#include "ScaleWaterExample.h"
#include "LineIntegralConvolution.h"

#include <iostream>
#include <memory>
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
	glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);

	auto colour = glm::vec4{ 99.0f,96.0f,93.0f,255.0f } / glm::vec4(255.0f);
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

    glfwTerminate();
}
