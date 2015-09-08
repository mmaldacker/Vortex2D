//
//  Reader.h
//  Vertex2D
//
//  Created by Maximilian Maldacker on 19/08/2015.
//  Copyright (c) 2015 Maximilian Maldacker. All rights reserved.
//

#ifndef __Vertex2D__Reader__
#define __Vertex2D__Reader__

#include "Common.h"
#include "Quad.h"
#include "RenderTexture.h"
#include "hfloat.h"

namespace Renderer
{

class Reader
{
public:
    Reader(Renderer::RenderTexture & texture);
    Reader(Reader &&);
    ~Reader();

    Reader & Read();
    Reader & Print();
    Reader & PrintStencil();

    float GetFloat(int x, int y);
    glm::vec2 GetVec2(int x, int y);
    glm::vec4 GetVec4(int x, int y);

private:
    int GetSize() const;
    float Get(int x, int y, int size, int offset);

    Renderer::RenderTexture & mTexture;
    float * mPixels;
    uint8_t * mStencil;
};

}

#endif /* defined(__Vertex2D__Reader__) */
