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
    , mPixels(new hfloat[(int)quad.Size().x*(int)quad.Size().y*2])
{
}

Reader::~Reader()
{
    delete [] mPixels;
}

void Reader::Read()
{
    GLenum format;
    switch(mTexture.GetFormat())
    {
        case Texture::PixelFormat::RF:
            format = GL_RED;
            break;
        case Texture::PixelFormat::RGF:
            format = GL_RG;
            break;
        case Texture::PixelFormat::RGBAF:
            format = GL_RGBA;
            break;
        default:
            throw std::runtime_error("unsupported read format");
            break;
    }

    mTexture.begin();
    glReadPixels(0, 0, mQuad.Size().x, mQuad.Size().y, format,GL_HALF_FLOAT, mPixels);
    mTexture.end();
}

float Reader::GetFloat(int x, int y)
{
    return Get(x, y, 1, 0);
}

glm::vec2 Reader::GetVec2(int x, int y)
{
    return {Get(x,y,2,0), Get(x,y,2,1)};
}

glm::vec4 Reader::GetVec4(int x, int y)
{
    return {Get(x,y,4,0), Get(x,y,4,1), Get(x,y,4,2), Get(x,y,4,3)};
}

float Reader::Get(int x, int y, int size, int offset)
{
    assert(x >=0 && x < mQuad.Size().x);
    assert(y >=0 && y < mQuad.Size().y);

    int width = mQuad.Size().x;
    return convertHFloatToFloat(mPixels[(x+y*width)*size+offset]);
}

}