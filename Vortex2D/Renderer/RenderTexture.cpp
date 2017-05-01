//
//  RenderTexture.cpp
//  Vortex
//

#include "RenderTexture.h"

#include <algorithm>
#include <string>
#include <stdexcept>

namespace Vortex2D { namespace Renderer {

RenderTexture::RenderTexture(int width, int height, Texture::PixelFormat pixelFormat, DepthFormat depthFormat)
    : Texture(width, height, pixelFormat)
    , RenderTarget(width, height)
{

}

RenderTexture::~RenderTexture()
{

}

RenderTexture::RenderTexture(RenderTexture&& other)
    : Texture(std::move(other))
    , RenderTarget(std::move(other))
{
}

RenderTexture& RenderTexture::operator=(RenderTexture&& other)
{
    Texture::operator=(std::move(other));
    RenderTarget::operator=(std::move(other));

    return *this;
}

void RenderTexture::ClearStencil()
{

}

void RenderTexture::BindBuffer()
{
}


}}
