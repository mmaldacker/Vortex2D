#include <SDL2/SDL.h>
#include "Common.h"
#include "ResourcePath.h"
#include "GridVisualiser.h"
#include "WindowRenderer.h"
#include "Text.h"

#include <string>

int main(int argc, const char * argv[])
{
    SDL_Init(SDL_INIT_VIDEO);

    int size = 32;
    int scale = 30;

    SDL_Log("My resource path is %s", getResourcePath().c_str());

    /*WindowRenderer window({size*scale,size*scale});
    window.SetBackgroundColour({1.0, 0.0, 0.0, 0.0});

    Text text;
    auto textSprite = text.Render("3.14159");
    window.AddDrawable(textSprite);*/

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

    SDL_Window * window = SDL_CreateWindow(NULL, 0, 0, size*scale, size*scale, SDL_WINDOW_OPENGL);
    SDL_GLContext context = SDL_GL_CreateContext(window);
    if(!context)
    {
        throw std::runtime_error(SDL_GetError());
    }


    GridVisualiser grid(window, context, {size, size}, scale);

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

        //window.Render();
        grid.Render();
    }

    SDL_Quit();
}
