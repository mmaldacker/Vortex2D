//
//  GLFW.cpp
//  Vortex2D
//

#include "glfw.h"

#include <stdexcept>
#include <string>

void ErrorCallback(int error, const char* description)
{
    throw std::runtime_error("GLFW Error: " +
                             std::to_string(error) +
                             " What: " +
                             std::string(description));
}

GLFWApp::GLFWApp(int width, int height, bool visible)
{
    glfwSetErrorCallback(ErrorCallback);

    if (!glfwInit())
    {
        throw std::runtime_error("Could not initialise GLFW!");
    }

    glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);
    glfwWindowHint(GLFW_VISIBLE, visible ? GLFW_TRUE : GLFW_FALSE);

   mWindow = glfwCreateWindow(width, height, "Vortex2D App", nullptr, nullptr);
   if (!mWindow)
   {
       throw std::runtime_error("Error creating GLFW Window");
   }

}

GLFWApp::~GLFWApp()
{
    glfwDestroyWindow(mWindow);
    glfwTerminate();
}

void GLFWApp::Run()
{
    while (!glfwWindowShouldClose(mWindow))
    {
        glfwPollEvents();
    }
}

void GLFWApp::SetKeyCallback(GLFWkeyfun cbfun)
{
    glfwSetKeyCallback(mWindow, cbfun);
}
