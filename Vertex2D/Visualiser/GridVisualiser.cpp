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

GridVisualiser::GridVisualiser(const glm::vec2 & size, int scale) : WindowRenderer(size*glm::vec2{scale})
    , mSize(size)
    , mScale(scale)
{
    SetBackgroundColour({1.0f, 1.0f, 1.0f, 1.0f});

    for(int i = 0 ; i < mSize.x ; ++i)
    {
        Renderer::Rectangle rect({2, mSize.y * mScale});
        rect.Position = glm::vec2{ i * mScale, 0 };
        rect.Colour = {0.0f, 0.0f, 0.0f, 1.0f};

        mGrid.push_back(std::move(rect));
    }

    for(int i = 0 ; i < mSize.y ; ++i)
    {
        Renderer::Rectangle rect({mSize.x * mScale,2});
        rect.Position = glm::vec2{0, i * mScale };
        rect.Colour = {0.0f, 0.0f, 0.0f, 1.0f};

        mGrid.push_back(std::move(rect));
    }
}

void GridVisualiser::RenderValue(const glm::vec2 & size, float value)
{
    auto text = mFont.Render(std::to_string(value));
    text.Position = size * glm::vec2{mScale};
    text.Scale = {1.0f, 0.8f * (float)mScale / mFont.GetCharacterHeight()};

    mValues.push_back(std::move(text));
}

void GridVisualiser::Render()
{
    Clear();
    WindowRenderer::Render(mGrid);
    WindowRenderer::Render(mValues);
    Swap();
}
