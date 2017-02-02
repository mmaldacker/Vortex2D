//
//  Texture.cpp
//  Vortex2D
//

#include "Texture.h"

namespace Vortex2D { namespace Renderer {

#include <cassert>
#include <stdexcept>

thread_local GLuint Texture::BoundId[4] = {0};
thread_local int Texture::ActiveUnit = -1;

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

    Unbind();
}

Texture::~Texture()
{
    if (mId)
    {
        // make sure we clear the cache for this texture id
        // because OpenGL can re-use texture ids and it won't be correctly bound
        for (auto& id : BoundId)
        {
            if (id == mId)
            {
                id = 0;
            }
        }

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

void Texture::SetAliasTexParameters()
{
    Bind();
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    Unbind();
}

void Texture::SetAntiAliasTexParameters()
{
    Bind();
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    Unbind();
}

void Texture::SetClampToEdgeTexParameters()
{
    Bind();
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    Unbind();
}

void Texture::SetClampToBorderTexParameters()
{
    Bind();
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    Unbind();
}

void Texture::Bind(int textureUnit) const
{
    if (ActiveUnit != textureUnit)
    {
        ActiveUnit = textureUnit;
        glActiveTexture(GL_TEXTURE0+textureUnit);
    }
    if (BoundId[textureUnit] != mId)
    {
        BoundId[textureUnit] = mId;
        glBindTexture(GL_TEXTURE_2D, mId);
    }
}

void Texture::Unbind()
{
    assert(ActiveUnit >= 0);
    BoundId[ActiveUnit] = 0;
    glBindTexture(GL_TEXTURE_2D, 0);
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
