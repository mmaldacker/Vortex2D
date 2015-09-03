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
    Reader(const Renderer::Quad & quad, Renderer::RenderTexture & texture);
    ~Reader();

    void Read();

    float GetFloat(int x, int y);
    glm::vec2 GetVec2(int x, int y);
    glm::vec4 GetVec4(int x, int y);

private:
    float Get(int x, int y, int size, int offset);

    const Renderer::Quad & mQuad;
    Renderer::RenderTexture & mTexture;
    hfloat * mPixels;
};

}

#endif /* defined(__Vertex2D__Reader__) */
