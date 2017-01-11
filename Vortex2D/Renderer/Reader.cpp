//
//  Reader.cpp
//  Vortex2D
//

#include "Reader.h"
#include <iostream>

namespace Vortex2D { namespace Renderer {

Reader::Reader(Renderer::RenderTexture & texture)
    : mTexture(texture)
{
    int size = GetSize();

    mPixels = new float[texture.Height() * texture.Width() * size];
    mStencil = new uint8_t[texture.Height() * texture.Width()];
}

Reader::~Reader()
{
    if (mPixels)
    {
        delete [] mPixels;
        delete [] mStencil;
    }
}

Reader::Reader(Reader && other) : mTexture(other.mTexture), mPixels(other.mPixels), mStencil(other.mStencil)
{
    other.mPixels = nullptr;
    other.mStencil = nullptr;
}

int Reader::GetSize() const
{
    switch (mTexture.GetFormat())
    {
        case Texture::PixelFormat::RF:
            return 1;
        case Texture::PixelFormat::RGF:
            return 2;
        case Texture::PixelFormat::RGBA8888:
        case Texture::PixelFormat::RGBAF:
            return 4;
        default:
            throw std::runtime_error("unsupported read format");
    }
}

Reader & Reader::Read()
{
    GLenum format;
    switch (mTexture.GetFormat())
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
    }

    mTexture.Begin();
    glReadPixels(0, 0, mTexture.Width(), mTexture.Height(), format, GL_FLOAT, mPixels);
    glReadPixels(0, 0, mTexture.Width(), mTexture.Height(), GL_STENCIL_INDEX, GL_UNSIGNED_BYTE, mStencil);
    mTexture.End();

    glFlush();

    return *this;
}

Reader & Reader::Print()
{
    assert(mPixels);

    int size = GetSize();
    for (int i = 0 ; i < mTexture.Width() ; ++i)
    {
        for (int j = 0 ; j < mTexture.Height() ; ++j)
        {
            std::cout << "(";
            int width = mTexture.Width();
            for (int k = 0 ; k < size ; k++)
            {
                std::cout << mPixels[i*width*size + j*size + k];
                if (k < size-1) std::cout << ",";
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
    assert(mStencil);

    for (int i = 0 ; i < mTexture.Width() ; ++i)
    {
        for (int j = 0 ; j < mTexture.Height() ; ++j)
        {
            int width = mTexture.Width();
            std::cout << "(" << (int)mStencil[i * width + j] << ")";
        }
        std::cout << std::endl;
    }
    std::cout << std::endl;

    return *this;
}

float Reader::GetFloat(int x, int y) const
{
    return Get(x, y, 1, 0);
}

glm::vec2 Reader::GetVec2(int x, int y) const
{
    return {Get(x, y, 2, 0), Get(x, y, 2, 1)};
}

glm::vec4 Reader::GetVec4(int x, int y) const
{
    return {Get(x, y, 4, 0), Get(x, y, 4, 1), Get(x, y, 4, 2), Get(x, y, 4, 3)};
}

float Reader::Get(int x, int y, int size, int offset) const
{
    assert(x >= 0 && x < mTexture.Width());
    assert(y >= 0 && y < mTexture.Height());
    assert(mPixels);

    int width = mTexture.Width();
    return mPixels[(x + y * width) * size + offset];
}

}}
