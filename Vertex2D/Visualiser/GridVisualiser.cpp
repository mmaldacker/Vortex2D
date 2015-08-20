//
//  GridVisualiser.cpp
//  Vertex2D
//
//  Created by Maximilian Maldacker on 19/08/2015.
//  Copyright (c) 2015 Maximilian Maldacker. All rights reserved.
//

#include "GridVisualiser.h"
#include <sstream>
#include <iomanip>

GridVisualiser::GridVisualiser(const glm::vec2 & size, int scale) : mSize(size), mScale(scale)
{
    mFont = TTF_OpenFont("/Library/Fonts/Verdana.ttf",10);
    if(!mFont)
    {
        throw std::runtime_error(TTF_GetError());
    }
    mWindow = SDL_CreateWindow(NULL, 0, 0, size.x*scale, size.y*scale, 0);
    mRenderer = SDL_CreateRenderer(mWindow, -1, SDL_RENDERER_ACCELERATED);
}

GridVisualiser::~GridVisualiser()
{
    TTF_CloseFont(mFont);
    SDL_DestroyRenderer(mRenderer);
    SDL_DestroyWindow(mWindow);
}

void GridVisualiser::RenderGrid()
{
    SDL_SetRenderDrawColor(mRenderer, 0xFF, 0xFF, 0xFF, 0xFF);
    SDL_RenderClear(mRenderer);

    SDL_SetRenderDrawColor(mRenderer, 0x00, 0x00, 0x00, 0xFF);

    for(int i = 0 ; i < mSize.x ; ++i)
    {
        SDL_Rect fillRect = {i * mScale, 0, 1, (int)mSize.y * mScale};
        SDL_RenderFillRect(mRenderer, &fillRect);
    }

    for(int i = 0 ; i < mSize.y ; ++i)
    {
        SDL_Rect fillRect = {0, i * mScale, (int)mSize.x * mScale, 1};
        SDL_RenderFillRect(mRenderer, &fillRect);
    }
}

void GridVisualiser::RenderValue(const glm::vec2 & size, float value)
{
    std::stringstream number;
    number << std::setprecision(2) << value;

    SDL_Texture * t = RenderText(number.str(), {0x00, 0x00, 0x00, 0xFF});
    glm::vec2 pos = size * mSize;
    int w, h;
    SDL_QueryTexture(t, NULL, NULL, &w, &h);
    SDL_Rect r{(int)pos.x, (int)pos.y, w, h};
    SDL_RenderCopy(mRenderer, t, NULL, &r);
    SDL_DestroyTexture(t);
}

void GridVisualiser::Render()
{
    SDL_RenderPresent(mRenderer);
}

SDL_Texture* GridVisualiser::RenderText(const std::string &message, SDL_Color color)
{
    SDL_Surface *surf = TTF_RenderText_Blended(mFont, message.c_str(), color);
    SDL_Texture *texture = SDL_CreateTextureFromSurface(mRenderer, surf);
    SDL_FreeSurface(surf);
    return texture;
}