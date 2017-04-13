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
    // FIXME need to change code depending if it's float or uint8
    // templating?

    int size = texture.GetNumberComponents();

    mStencil = new uint8_t[texture.Height() * texture.Width()];

    glGenBuffers(1, &mPixelBuffer);
    glBindBuffer(GL_PIXEL_PACK_BUFFER, mPixelBuffer);

    int bufferSize = size * 4 * texture.Width() * texture.Height();
    glBufferData(GL_PIXEL_PACK_BUFFER, bufferSize, NULL, GL_STREAM_READ);

    glBindBuffer(GL_PIXEL_PACK_BUFFER, 0);
}

Reader::Reader(Buffer& buffer)
    : Reader(buffer.mTextures.front())
{

}

Reader::~Reader()
{
    if (mStencil)
    {
        delete [] mStencil;
    }

    if (mPixelBuffer)
    {
        glDeleteBuffers(1, &mPixelBuffer);
    }
}

Reader::Reader(Reader&& other)
    : mTexture(other.mTexture)
    , mPixelBuffer(other.mPixelBuffer)
    , mStencil(other.mStencil)
{
    other.mPixelBuffer = 0;
    other.mStencil = nullptr;
}

Reader& Reader::Read()
{
    EnableParameter p(glPixelStorei, GL_PACK_ALIGNMENT, 1);

    mTexture.BindBuffer();
    glBindBuffer(GL_PIXEL_PACK_BUFFER, mPixelBuffer);
    glReadPixels(0, 0, mTexture.Width(), mTexture.Height(), mTexture.GetFormat(), GL_FLOAT, NULL);
    glBindBuffer(GL_PIXEL_PACK_BUFFER, 0);

    return *this;
}

Reader& Reader::ReadStencil()
{
    EnableParameter p(glPixelStorei, GL_PACK_ALIGNMENT, 1);

    mTexture.BindBuffer();
    glReadPixels(0, 0, mTexture.Width(), mTexture.Height(), GL_STENCIL_INDEX, GL_UNSIGNED_BYTE, mStencil);

    glFlush();

    return *this;
}

Reader& Reader::Print()
{
    glBindBuffer(GL_PIXEL_PACK_BUFFER, mPixelBuffer);
    const float* pixels = (const float*)glMapBuffer(GL_PIXEL_PACK_BUFFER, GL_READ_ONLY);
    assert(pixels);

    int size = mTexture.GetNumberComponents();
    for (int j = 0 ; j < mTexture.Height() ; ++j)
    {
        for (int i = 0 ; i < mTexture.Width() ; ++i)
        {
            std::cout << "(";
            int width = mTexture.Width();
            for (int k = 0 ; k < size ; k++)
            {
                std::cout << pixels[i * size + j * width * size + k];
                if (k < size - 1) std::cout << ",";
            }
            std::cout << ")";
        }
        std::cout << std::endl;
    }
    std::cout << std::endl;

    glUnmapBuffer(GL_PIXEL_PACK_BUFFER);
    glBindBuffer(GL_PIXEL_PACK_BUFFER, 0);

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

    // FIXME this is not very efficitent to call for every Get
    glBindBuffer(GL_PIXEL_PACK_BUFFER, mPixelBuffer);
    const float* pixels = (const float*)glMapBuffer(GL_PIXEL_PACK_BUFFER, GL_READ_ONLY);
    assert(pixels);

    int width = mTexture.Width();
    float value = pixels[(x + y * width) * size + offset];

    glUnmapBuffer(GL_PIXEL_PACK_BUFFER);
    glBindBuffer(GL_PIXEL_PACK_BUFFER, 0);

    return value;
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
