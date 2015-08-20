//
//  Texture.cpp
//  Vortex
//
//  Created by Maximilian Maldacker on 06/04/2014.
//
//

#include "Texture.h"

namespace Renderer
{

#include <cassert>
#include <stdexcept>

int Texture::BoundId[4] = {-1};
int Texture::ActiveUnit = -1;

Texture::Texture(int width, int height, PixelFormat pixelFormat, void * data)
: mWidth(width), mHeight(height), mOffsetX(0), mOffsetY(0)
{
    assert(mWidth > 0);
    assert(mHeight > 0);
    
    if (supports_npot_textures())
    {
        mStoredWidth = mWidth;
        mStoredHeight = mHeight;
    } else
    {
        mStoredWidth = next_power_of_two(mWidth);
        mStoredHeight = next_power_of_two(mHeight);
    }

    /*
     In the absence of OES_texture_npot, which lifts these restrictions, neither
     mipmapping nor wrap modes other than CLAMP_TO_EDGE are supported in
     conjunction with NPOT 2D textures.  A NPOT 2D texture with a wrap mode that
     is not CLAMP_TO_EDGE or a minfilter that is not NEAREST or LINEAR is
     considered incomplete.  If such a texture is bound to a texture unit, it is
     as if texture mapping were disabled for that texture unit.
     */

    glGenTextures(1, &mId);
    Bind(0);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    switch(pixelFormat)
    {
        case PixelFormat::RGBA8888:
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, (GLsizei) width, (GLsizei) height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
            break;
        case PixelFormat::RGB565:
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, (GLsizei) width, (GLsizei) height, 0, GL_RGB, GL_UNSIGNED_SHORT_5_6_5, data);
            break;
        case PixelFormat::RGBAF:
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, (GLsizei) width, (GLsizei) height, 0, GL_RGBA, GL_HALF_FLOAT, data);
            break;
        case PixelFormat::RGBF:
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, (GLsizei) width, (GLsizei) height, 0, GL_RGB, GL_HALF_FLOAT, data);
            break;
        case PixelFormat::RGF:
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RG, (GLsizei) width, (GLsizei) height, 0, GL_RG, GL_HALF_FLOAT, data);
            break;
        case PixelFormat::RF:
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, (GLsizei) width, (GLsizei) height, 0, GL_RED, GL_HALF_FLOAT, data);
            break;
        default:
            throw std::runtime_error("Unkown pixel format");
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
    , mStoredWidth(other.mStoredWidth)
    , mStoredHeight(other.mStoredHeight)
    , mOffsetX(other.mOffsetX)
    , mOffsetY(other.mOffsetY)
{
    other.mId = 0;
}

Texture & Texture::operator=(Texture && other)
{
    mId = other.mId;
    mWidth = other.mWidth;
    mHeight = other.mHeight;
    mStoredWidth = other.mStoredWidth;
    mStoredHeight = other.mStoredHeight;
    mOffsetX = other.mOffsetX;
    mOffsetY = other.mOffsetY;

    other.mId = 0;

    return *this;
}

void Texture::SetAliasTexParameters()
{
    Bind(0);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    Unbind();
}

void Texture::SetAntiAliasTexParameters()
{
    Bind(0);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    Unbind();
}

void Texture::Bind(int textureUnit)
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
    BoundId[ActiveUnit] = -1;
    glBindTexture(GL_TEXTURE_2D, 0);
}

}
