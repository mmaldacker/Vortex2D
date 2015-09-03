//
//  Size.h
//  Vertex2D
//
//  Created by Maximilian Maldacker on 01/09/2015.
//  Copyright (c) 2015 Maximilian Maldacker. All rights reserved.
//

#ifndef Vertex2D_Size_h
#define Vertex2D_Size_h

#include "Common.h"

namespace Fluid
{

struct Dimensions
{
    Dimensions(const glm::vec2 & size, float scale) : Scale(scale), Size(glm::floor(size/scale))
    {
        InvScale = glm::scale(glm::vec3{1.0f/Scale, 1.0f/Scale, 1.0f});
        InvScale = glm::translate(InvScale, glm::vec3{-Scale*0.5f, -Scale*0.5f, 0.0f});
    }
    
    int Scale;
    glm::vec2 Size;
    glm::mat4x4 InvScale;
};

}

#endif
