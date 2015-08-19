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

    glm::vec2 GetInterpolatedVelocityAt(int x, int y);

private:
    glm::vec2 GetVelocityAt(int x, int y);

    const Renderer::Quad & mQuad;
    Renderer::RenderTexture & mTexture;
    hfloat * mPixels;
};

}

#endif /* defined(__Vertex2D__Reader__) */
