//
//  Buffer.cpp
//  Vortex2D
//

#include "Buffer.h"

#include <Vortex2D/Renderer/Shader.h>
#include <Vortex2D/Renderer/Operator.h>

namespace Vortex2D { namespace Renderer {

Buffer::Buffer(const glm::vec2 & size, unsigned components, bool doubled, bool depth)
    : RenderTarget(size.x, size.y)
    , mSprite(size)
{
    Add(size, components, depth);
    if (doubled) Add(size, components, depth);
}

Buffer& Buffer::operator=(OperatorContext context)
{
    mSprite.NoTexture();
    mSprite.SetProgram(context.Program);
    Render(mSprite);
    return *this;
}

void Buffer::ClearStencil()
{
    for (auto&& t : mTextures) t.ClearStencil();
}

void Buffer::Linear()
{
    for (auto&& t : mTextures) t.Linear();
}

void Buffer::ClampToEdge()
{
    for (auto&& t : mTextures) t.ClampToEdge();
}

void Buffer::Clear(const glm::vec4& colour)
{
    for (auto&& t : mTextures) t.Clear(colour);
}

void Buffer::Render(Renderer::Drawable& object, const glm::mat4& transform)
{
    mTextures.front().Render(object, transform);
}

void Buffer::Swap()
{
    assert(mTextures.size() == 2);
    std::swap(mTextures.front(), mTextures.back());
}

Renderer::Sprite& Buffer::Sprite()
{
    mSprite.SetTexture(mTextures.front());
    return mSprite;
}

bool Buffer::IsDoubleBuffer() const
{
    return mTextures.size() == 2;
}

int Buffer::Width() const
{
    return mTextures.front().Width();
}

int Buffer::Height() const
{
    return mTextures.front().Height();
}

void Buffer::Add(const glm::vec2& size, unsigned components, bool depth)
{
    mTextures.emplace_back(size.x, size.y,
                           components == 1 ? Renderer::Texture::PixelFormat::RF :
                           components == 2 ? Renderer::Texture::PixelFormat::RGF :
                                             Renderer::Texture::PixelFormat::RGBAF,
                           depth ? Renderer::RenderTexture::DepthFormat::DEPTH24_STENCIL8 :
                                   Renderer::RenderTexture::DepthFormat::NONE);
    mTextures.back().Nearest();
    mTextures.back().ClampToBorder();
}

}}
