//
//  Reader.cpp
//  Vertex2D
//
//  Created by Maximilian Maldacker on 19/08/2015.
//  Copyright (c) 2015 Maximilian Maldacker. All rights reserved.
//

#include "Reader.h"

namespace Renderer
{

Reader::Reader(const Renderer::Quad & quad, Renderer::RenderTexture & texture)
    : mQuad(quad)
    , mTexture(texture)
    , mPixels(new hfloat[(int)quad.FullSize().x*(int)quad.FullSize().y*2])
{
}

Reader::~Reader()
{
    delete [] mPixels;
}

void Reader::Read()
{
    mTexture.begin();
    glReadPixels(0, 0, mQuad.Size().x, mQuad.Size().y, GL_RG,GL_HALF_FLOAT, mPixels);
    mTexture.end();
}

glm::vec2 Reader::GetInterpolatedVelocityAt(int x, int y)
{
    glm::vec2 xy = glm::vec2{x,y};

    glm::vec2 ij = glm::floor(xy);
    glm::vec2 kl = ij + glm::vec2(1.0);
    glm::vec2 f = xy - ij;

    glm::vec2 t11 = GetVelocityAt(ij.x, ij.y);
    glm::vec2 t21 = GetVelocityAt(ij.x, kl.y);
    glm::vec2 t12 = GetVelocityAt(kl.x, ij.y);
    glm::vec2 t22 = GetVelocityAt(kl.x, kl.y);

    return glm::mix(glm::mix(t11,t21,f.y),glm::mix(t12,t22,f.y),f.x);
}

glm::vec2 Reader::GetVelocityAt(int x, int y)
{
    assert(x >=0 && x < mQuad.Size().x);
    assert(y >=0 && y < mQuad.Size().y);

    int width = mQuad.Size().x;
    return {convertHFloatToFloat(mPixels[(x+y*width)*2]), convertHFloatToFloat(mPixels[(x+y*width)*2+1])};
}

}