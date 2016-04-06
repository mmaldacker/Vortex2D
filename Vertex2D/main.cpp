#include "Common.h"
#include "ResourcePath.h"
#include "SmokeTest.h"

void error_callback(int error, const char* description)
{
    std::cout << "GLFW erro " << error << " : " << description << std::endl;
}

int main(int argc, const char * argv[])
{
    if(!glfwInit()) { std::cout << "Could not initialize GLFW" << std::endl; exit(EXIT_FAILURE); }

    glfwSetErrorCallback(error_callback);

    std::cout << "My resource path is " << getResourcePath() << std::endl;

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    SmokeExample().run();

    CHECK_GL_ERROR_DEBUG();

    glfwTerminate();
}
