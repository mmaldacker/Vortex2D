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
    int size = GetSize();

    mPixels = new float[texture.StoredHeight()*texture.StoredWidth()*size];
    mStencil = new uint8_t[texture.StoredHeight()*texture.StoredWidth()];
}

Reader::~Reader()
{
    if(mPixels)
    {
        delete [] mPixels;
        delete [] mStencil;
    }
}

Reader::Reader(Reader && other) : mTexture(other.mTexture), mPixels(other.mPixels), mStencil(other.mStencil)
{
    other.mPixels = nullptr;
}

int Reader::GetSize() const
{
    switch(mTexture.GetFormat())
    {
        case Texture::PixelFormat::RF:
            return 1;
            break;
        case Texture::PixelFormat::RGF:
            return 2;
            break;
        case Texture::PixelFormat::RGBA8888:
        case Texture::PixelFormat::RGBAF:
            return 4;
            break;
        default:
            throw std::runtime_error("unsupported read format");
            break;
    }
}

Reader & Reader::Read()
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
        case Texture::PixelFormat::RGBA8888:
        case Texture::PixelFormat::RGBAF:
            format = GL_RGBA;
            break;
        default:
            throw std::runtime_error("unsupported read format");
            break;
    }

    mTexture.begin();
    glReadPixels(0, 0, mTexture.StoredWidth(), mTexture.StoredHeight(), format, GL_FLOAT, mPixels);
    glReadPixels(0, 0, mTexture.StoredWidth(), mTexture.StoredHeight(), GL_STENCIL_INDEX, GL_UNSIGNED_BYTE, mStencil);
    mTexture.end();

    glFlush();

    return *this;
}

Reader & Reader::Print()
{
    int size = GetSize();
    for(int i = 0 ; i < mTexture.StoredWidth() ; ++i)
    {
        for(int j = 0 ; j < mTexture.StoredHeight() ; ++j)
        {
            std::cout << "(";
            int width = mTexture.StoredWidth();
            for(int k = 0 ; k < size ; k++)
            {
                std::cout << mPixels[i*width*size + j*size + k];
                if(k < size-1) std::cout << ",";
            }
            std::cout << ")";
        }
        std::cout << std::endl;
    }
    std::cout << std::endl;

    return *this;
}

Reader & Reader::PrintStencil()
{
    for(int i = 0 ; i < mTexture.StoredWidth() ; ++i)
    {
        for(int j = 0 ; j < mTexture.StoredHeight() ; ++j)
        {
            int width = mTexture.StoredWidth();
            std::cout << "(" << (int)mStencil[i*width + j] << ")";
        }
        std::cout << std::endl;
    }

    return *this;
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
    return mPixels[(x+y*width)*size+offset];
}

}