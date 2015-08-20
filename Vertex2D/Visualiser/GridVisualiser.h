//
//  GridVisualiser.h
//  Vertex2D
//
//  Created by Maximilian Maldacker on 19/08/2015.
//  Copyright (c) 2015 Maximilian Maldacker. All rights reserved.
//

#ifndef __Vertex2D__GridVisualiser__
#define __Vertex2D__GridVisualiser__

#include "Common.h"
#include <SDL2/SDL.h>
#include <SDL2_ttf/SDL_ttf.h>
#include <string>

class GridVisualiser
{
public:
    GridVisualiser(const glm::vec2 & size, int scale);
    ~GridVisualiser();

    void RenderGrid();
    void RenderValue(const glm::vec2 & size, float value);
    void Render();

private:
    SDL_Texture* RenderText(const std::string &message, SDL_Color color);

    glm::vec2 mSize;
    int mScale;
    SDL_Window * mWindow;
    SDL_Renderer * mRenderer;
    TTF_Font * mFont;
};

#endif /* defined(__Vertex2D__GridVisualiser__) */
