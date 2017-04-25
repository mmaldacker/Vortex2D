//
//  Reader.cpp
//  Vortex2D
//

#include "Reader.h"


#include <iostream>

namespace Vortex2D { namespace Renderer {

Reader::Reader(RenderTexture& texture)
 : mTexture(texture)
{
}

Reader::Reader(Buffer& buffer)
    : Reader(buffer.mTextures.front())
{

}

Reader::~Reader()
{
}

Reader::Reader(Reader&& other) : mTexture(other.mTexture)
{
}

Reader& Reader::Read()
{

    return *this;
}

Reader& Reader::Print()
{
    const float* pixels = nullptr;
    assert(pixels);

    int size = mTexture.GetNumberComponents();
    for (int j = 0 ; j < mTexture.Height ; ++j)
    {
        for (int i = 0 ; i < mTexture.Width ; ++i)
        {
            std::cout << "(";
            int width = mTexture.Width;
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
    assert(x >= 0 && x < mTexture.Width);
    assert(y >= 0 && y < mTexture.Height);
    return 0.0f;
}

int Reader::Width() const
{
    return mTexture.Width;
}

int Reader::Height() const
{
    return mTexture.Height;
}

}}
