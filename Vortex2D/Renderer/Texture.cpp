//
//  Texture.cpp
//  Vortex2D
//

#include "Texture.h"

namespace Vortex2D { namespace Renderer {

#include <cassert>
#include <stdexcept>

int Texture::BoundId[4] = {0};
int Texture::ActiveUnit = -1;

Texture::Texture(int width, int height, PixelFormat pixelFormat, const void * data)
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

    // FIXME temporary, real solution is handle alignemnt in reader (don't think anywhere else)
    glPixelStorei(GL_PACK_ALIGNMENT, 1);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    switch(pixelFormat)
    {
        case PixelFormat::RGBA8888:
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, (GLsizei) width, (GLsizei) height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
            break;
        case PixelFormat::RGB888:
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, (GLsizei) width, (GLsizei) height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
            break;
        case PixelFormat::RGBAF:
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, (GLsizei) width, (GLsizei) height, 0, GL_RGBA, GL_FLOAT, data);
            break;
        case PixelFormat::RGBF:
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, (GLsizei) width, (GLsizei) height, 0, GL_RGB, GL_FLOAT, data);
            break;
        case PixelFormat::RGF:
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RG32F, (GLsizei) width, (GLsizei) height, 0, GL_RG, GL_FLOAT, data);
            break;
        case PixelFormat::RF:
            glTexImage2D(GL_TEXTURE_2D, 0, GL_R32F, (GLsizei) width, (GLsizei) height, 0, GL_RED, GL_FLOAT, data);
            break;
    }

    Unbind();
}

Texture::~Texture()
{
    if (mId)
    {
        glDeleteTextures(1, &mId);
    }
}

Texture::Texture(Texture && other)
    : mId(other.mId)
    , mWidth(other.mWidth)
    , mHeight(other.mHeight)
    , mFormat(other.mFormat)
{
    other.mId = 0;
}

Texture & Texture::operator=(Texture && other)
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
    if(ActiveUnit != textureUnit)
    {
        ActiveUnit = textureUnit;
        glActiveTexture(GL_TEXTURE0+textureUnit);
    }
    if(BoundId[textureUnit] != mId)
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

Texture::PixelFormat Texture::GetFormat() const
{
    return mFormat;
}

}}
