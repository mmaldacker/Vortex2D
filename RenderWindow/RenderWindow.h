//
//  RenderWindow.h
//  Vortex2D
//

#ifndef RenderWindow_h
#define RenderWindow_h

#include <Vortex2D/Renderer/RenderTarget.h>
#include <GLFW/glfw3.h>
#include <string>

class RenderWindow : public Vortex2D::Renderer::RenderTarget
{
public:
    RenderWindow(int width,
                 int height,
                 const std::string & name = "Window",
                 RenderWindow * share = nullptr);

    virtual ~RenderWindow();

    void MakeCurrent();
    void Clear(const glm::vec4 & colour);
    void Render(Vortex2D::Renderer::Drawable & object, const glm::mat4 & transform);
    void Display();
    bool ShouldClose();
    void SetKeyCallback(GLFWkeyfun cbfun);

private:
    int mWidth, mHeight;
    GLFWwindow * mWindow;
};

#endif
