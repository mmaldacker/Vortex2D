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
    Dimensions(const glm::vec2 & size, const float scale)
        : Scale(scale)
        , Size(glm::floor(size/scale))
        , InvScale(glm::scale(glm::vec3{1.0f/scale, 1.0f/scale, 1.0f}))
    {
    }
    
    const float Scale;
    const glm::vec2 Size;
    const glm::mat4 InvScale;
};

}

#endif
