//
//  Quad.h
//  Vertex2D
//
//  Created by Maximilian Maldacker on 19/08/2015.
//  Copyright (c) 2015 Maximilian Maldacker. All rights reserved.
//

#ifndef __Vertex2D__Quad__
#define __Vertex2D__Quad__

#include "Common.h"

namespace Renderer
{

class Quad
{
public:
    Quad(const glm::vec2 & size);
    ~Quad();

    Quad(Quad &&);

    void Render();

    const glm::vec2 & Size() const;

private:
    glm::vec2 mSize;
    GLuint mVertexArray;
    GLuint mVertexBuffer;
};

}

#endif /* defined(__Vertex2D__Quad__) */
