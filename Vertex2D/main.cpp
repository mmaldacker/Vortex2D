#include <SDL2/SDL.h>
#include "Common.h"
#include "ResourcePath.h"

#include <string>

int main(int argc, const char * argv[])
{
    if (SDL_Init(SDL_INIT_VIDEO) < 0)
    {
        printf("Could not initialize SDL\n");
        return 1;
    }

    int size = 64;
    int scale = 10;

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
    SDL_Window * sdlWindow = SDL_CreateWindow(NULL, 0, 0, size*scale, size*scale, SDL_WINDOW_OPENGL);
    if (!sdlWindow)
    {
        printf("Could not initialize Window\n");
        return 1;
    }

    SDL_GLContext gl = SDL_GL_CreateContext(sdlWindow);

    const GLubyte * c_str = glGetString(GL_EXTENSIONS);

    std::string extensions((const char *)c_str);
    std::replace(extensions.begin(), extensions.end(), ' ', '\n');

    SDL_Log("Extensions:\n%s", extensions.c_str());
    SDL_Log("My resource path is %s", getResourcePath().c_str());

    SDL_Event e;
    bool quit = false;
    while (!quit)
    {
        while (SDL_PollEvent(&e))
        {
            if (e.type == SDL_QUIT)
            {
                quit = true;
            }
        }
    }


    SDL_GL_DeleteContext(gl);
    SDL_DestroyWindow(sdlWindow);
    SDL_Quit();
}
