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
    TTF_Init();

    int size = 32;
    int scale = 30;

    SDL_Log("My resource path is %s", getResourcePath().c_str());

    WindowRenderer window({size*scale,size*scale});
    window.SetBackgroundColour({1.0, 0.0, 0.0, 0.0});

    Text text;
    auto textSprite = text.Render("Hello World!");
    window.AddDrawable(textSprite);

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

        window.Render();
    }

    SDL_Quit();
}
