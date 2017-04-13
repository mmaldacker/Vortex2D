//
//  Texture.cpp
//  Vortex2D
//

#include "Texture.h"

#include <Vortex2D/Renderer/State.h>

#include <cassert>

namespace Vortex2D { namespace Renderer {

Texture::Texture(int width, int height, PixelFormat pixelFormat)
    : mWidth(width), mHeight(height), mFormat(pixelFormat)
{
    assert(mWidth > 0);
    assert(mHeight > 0);

    glGenTextures(1, &mId);
    Bind();
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glTexImage2D(GL_TEXTURE_2D,
                 0,
                 GetInternalFormat(),
                 (GLsizei) width,
                 (GLsizei) height,
                 0,
                 GetFormat(),
                 GetType(),
                 NULL);
}

Texture::~Texture()
{
    if (mId)
    {
        State::ClearTexture(mId);
        glDeleteTextures(1, &mId);
    }
}

Texture::Texture(Texture&& other)
    : mId(other.mId)
    , mWidth(other.mWidth)
    , mHeight(other.mHeight)
    , mFormat(other.mFormat)
{
    other.mId = 0;
}

Texture& Texture::operator=(Texture&& other)
{
    mId = other.mId;
    mWidth = other.mWidth;
    mHeight = other.mHeight;
    mFormat = other.mFormat;

    other.mId = 0;

    return *this;
}

void Texture::Nearest()
{
    Bind();
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
}

void Texture::Linear()
{
    Bind();
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
}

void Texture::BorderColour(const glm::vec4& colour)
{
  Bind();
  glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, &colour[0]);
}

void Texture::ClampToEdge()
{
    Bind();
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
}

void Texture::ClampToBorder()
{
    Bind();
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
}

void Texture::Bind(int textureUnit) const
{
    State::BindTexture(mId, textureUnit);
}

GLint Texture::GetInternalFormat() const
{
    switch (mFormat)
    {
        case PixelFormat::RGBA8888:
            return GL_RGBA;
        case PixelFormat::RGB888:
            return GL_RGB;
        case PixelFormat::RGBAF:
            return GL_RGBA32F;
        case PixelFormat::RGBF:
            return GL_RGB32F;
        case PixelFormat::RGF:
            return GL_RG32F;
        case PixelFormat::RF:
            return GL_R32F;
    }
}

GLenum Texture::GetFormat() const
{
    switch (mFormat)
    {
        case PixelFormat::RGBA8888:
        case PixelFormat::RGBAF:
            return GL_RGBA;
        case PixelFormat::RGB888:
        case PixelFormat::RGBF:
            return GL_RGB;
        case PixelFormat::RGF:
            return GL_RG;
        case PixelFormat::RF:
            return GL_RED;
    }
}

GLenum Texture::GetType() const
{
    switch (mFormat)
    {
        case PixelFormat::RGBA8888:
        case PixelFormat::RGB888:
            return GL_UNSIGNED_BYTE;
        case PixelFormat::RGBAF:
        case PixelFormat::RGBF:
        case PixelFormat::RGF:
        case PixelFormat::RF:
            return GL_FLOAT;
    }
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
