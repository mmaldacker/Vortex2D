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
#include "WindowRenderer.h"
#include "Shapes.h"
#include <vector>

class GridVisualiser : public WindowRenderer
{
public:
    GridVisualiser(const glm::vec2 & size, int scale);

    void RenderValue(const glm::vec2 & size, float value);

private:
    glm::vec2 mSize;
    int mScale;

    std::vector<Renderer::Rectangle> mGrid;
};

#endif /* defined(__Vertex2D__GridVisualiser__) */
