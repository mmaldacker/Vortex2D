//
//  Texture.cpp
//  Vortex2D
//

#include "Texture.h"

#include <cassert>

namespace Vortex2D { namespace Renderer {

Texture::Texture(int width, int height, PixelFormat pixelFormat)
    : mFormat(pixelFormat)
{
}

Texture::~Texture()
{
}

Texture::Texture(Texture&& other)
    : mFormat(other.mFormat)
{
}

Texture& Texture::operator=(Texture&& other)
{
    mFormat = other.mFormat;

    return *this;
}

void Texture::Nearest()
{
    Bind();
}

void Texture::Linear()
{
    Bind();
}

void Texture::BorderColour(const glm::vec4& colour)
{
    Bind();
}

void Texture::ClampToEdge()
{
    Bind();
}

void Texture::ClampToBorder()
{
    Bind();
}

void Texture::Bind(int textureUnit) const
{
}

unsigned Texture::GetNumberComponents() const
{
    switch (mFormat)
    {
        case PixelFormat::RGBA8888:
        case PixelFormat::RGBAF:
            return 4;
        case PixelFormat::RGB888:
        case PixelFormat::RGBF:
            return 3;
        case PixelFormat::RGF:
            return 2;
        case PixelFormat::RF:
            return 1;
    }
}

}}
