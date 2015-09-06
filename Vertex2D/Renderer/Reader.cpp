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

Reader::Reader(Renderer::RenderTexture & texture)
    : mTexture(texture)
{
    int size;
    switch(mTexture.GetFormat())
    {
        case Texture::PixelFormat::RF:
            size = 1;
            break;
        case Texture::PixelFormat::RGF:
            size = 2;
            break;
        case Texture::PixelFormat::RGBAF:
            size = 4;
            break;
        default:
            throw std::runtime_error("unsupported read format");
            break;
    }

    mPixels = new float[texture.StoredHeight()*texture.StoredWidth()*size];
}

Reader::~Reader()
{
    if(mPixels)
    {
        delete [] mPixels;
    }
}

Reader::Reader(Reader && other) : mTexture(other.mTexture), mPixels(other.mPixels)
{
    other.mPixels = nullptr;
}

void Reader::Read()
{
    GLenum format;
    int size ;
    switch(mTexture.GetFormat())
    {
        case Texture::PixelFormat::RF:
            format = GL_RED;
            size = 1;
            break;
        case Texture::PixelFormat::RGF:
            format = GL_RG;
            size = 2;
            break;
        case Texture::PixelFormat::RGBAF:
            format = GL_RGBA;
            size = 4;
            break;
        default:
            throw std::runtime_error("unsupported read format");
            break;
    }

    mTexture.begin();
    glReadPixels(0, 0, mTexture.StoredWidth(), mTexture.StoredHeight(), format, GL_FLOAT, mPixels);
    mTexture.end();

    glFlush();

    for(int i = 0 ; i < mTexture.StoredWidth() ; ++i)
    {
        for(int j = 0 ; j < mTexture.StoredHeight() ; ++j)
        {
            std::cout << "(";
            int width = mTexture.StoredWidth();
            for(int k = 0 ; k < size ; k++)
            {
                //std::cout << convertHFloatToFloat(mPixels[i*width*size + j*size + k]) << ",";
                float value = mPixels[i*width*size + j*size + k];
                std::cout << value << ",";
            }
            std::cout << ")";
        }
        std::cout << std::endl;
    }
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
    assert(x >=0 && x < mTexture.StoredWidth());
    assert(y >=0 && y < mTexture.StoredHeight());

    int width = mTexture.StoredWidth();
    //return convertHFloatToFloat(mPixels[(x+y*width)*size+offset]);
    return mPixels[(x+y*width)*size+offset];
}

}