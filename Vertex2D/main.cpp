#include "Common.h"
#include "ResourcePath.h"
#include "GridVisualiser.h"
#include "WindowRenderer.h"
#include "Text.h"

#include <string>

void error_callback(int error, const char* description)
{
    std::cout << "GLFW erro " << error << " : " << description << std::endl;
}

int main(int argc, const char * argv[])
{
    if(!glfwInit()) { std::cout << "Could not initialize GLFW" << std::endl; exit(EXIT_FAILURE); }

    glfwSetErrorCallback(error_callback);

    int size = 30;
    int scale = 30;

    std::cout << "My resource path is " << getResourcePath() << std::endl;

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GridVisualiser grid({size, size}, scale);

    WindowRenderer window({size*scale,size*scale}, &grid);
    window.SetBackgroundColour({1.0, 0.0, 0.0, 0.0});

    Renderer::Rectangle rect(glm::vec2{40.0f, 40.0f});
    rect.Position = {100.0f, 100.0f};
    rect.Colour = {0.0f, 1.0f, 0.0f, 1.0f};

    Text text;
    auto textSprite = text.Render(std::to_string(3.657));
    textSprite.Position = glm::vec2{100.f};

    Renderer::Sprite font(text.GetFont());

    Renderer::TextureCoords c;
    c.tex = {glm::vec2{96.0f}, glm::vec2{32.0f}};
    c.pos = {glm::vec2{0.0f}, glm::vec2{32.0f}};
    Renderer::Sprite font2(text.GetFont(), Renderer::Quad{{text.GetFont().Width(), text.GetFont().Height()},c});
    //font2.Position = {100.0f, 100.0f};

    window.AddDrawable(rect);
    //window.AddDrawable(font);
    //window.AddDrawable(font2);
    window.AddDrawable(textSprite);

    while (!window.ShouldClose() && !grid.ShouldClose())
    {
        window.Render();
        grid.Render();

        glfwPollEvents();
    }

    glfwTerminate();
}
