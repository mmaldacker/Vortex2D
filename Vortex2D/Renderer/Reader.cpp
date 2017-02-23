//
//  Reader.cpp
//  Vortex2D
//

#include "Reader.h"

#include <Vortex2D/Renderer/Disable.h>

#include <iostream>

namespace Vortex2D { namespace Renderer {

Reader::Reader(RenderTexture& texture)
    : mTexture(texture)
{
    int size = texture.GetNumberComponents();

    mPixels = new float[texture.Height() * texture.Width() * size];
    mStencil = new uint8_t[texture.Height() * texture.Width()];
}

Reader::Reader(Buffer& buffer)
    : Reader(buffer.mTextures.front())
{

}

Reader::~Reader()
{
    if (mPixels)
    {
        delete [] mPixels;
        delete [] mStencil;
    }
}

Reader::Reader(Reader&& other) : mTexture(other.mTexture), mPixels(other.mPixels), mStencil(other.mStencil)
{
    other.mPixels = nullptr;
    other.mStencil = nullptr;
}

Reader& Reader::Read()
{
    EnableParameter p(glPixelStorei, GL_PACK_ALIGNMENT, 1);

    mTexture.Begin();
    glReadPixels(0, 0, mTexture.Width(), mTexture.Height(), mTexture.GetFormat(), GL_FLOAT, mPixels);
    glReadPixels(0, 0, mTexture.Width(), mTexture.Height(), GL_STENCIL_INDEX, GL_UNSIGNED_BYTE, mStencil);
    mTexture.End();

    glFlush();

    return *this;
}

Reader& Reader::Print()
{
    assert(mPixels);

    int size = mTexture.GetNumberComponents();
    for (int j = 0 ; j < mTexture.Height() ; ++j)
    {
        for (int i = 0 ; i < mTexture.Width() ; ++i)
        {
            std::cout << "(";
            int width = mTexture.Width();
            for (int k = 0 ; k < size ; k++)
            {
                std::cout << mPixels[i * size + j * width * size + k];
                if (k < size - 1) std::cout << ",";
            }
            std::cout << ")";
        }
        std::cout << std::endl;
    }
    std::cout << std::endl;

    return *this;
}

Reader& Reader::PrintStencil()
{
    assert(mStencil);

    for (int j = 0 ; j < mTexture.Height() ; ++j)
    {
        for (int i = 0 ; i < mTexture.Width() ; ++i)
        {
            int width = mTexture.Width();
            std::cout << "(" << (int)mStencil[i + j * width] << ")";
        }
        std::cout << std::endl;
    }
    std::cout << std::endl;

    return *this;
}

uint8_t Reader::GetStencil(int x, int y) const
{
    assert(x >= 0 && x < mTexture.Width());
    assert(y >= 0 && y < mTexture.Height());
    assert(mStencil);

    int width = mTexture.Width();
    return mStencil[x + y * width];
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

int Reader::Width() const
{
    return mTexture.Width();
}

int Reader::Height() const
{
    return mTexture.Height();
}

}}
