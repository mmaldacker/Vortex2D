//
//  Buffer.cpp
//  Vortex2D
//
//  Created by Maximilian Maldacker on 15/06/2016.
//
//

#include "Buffer.h"
#include "Shader.h"
#include "Operator.h"

namespace Fluid
{

Quad::Quad(const glm::vec2 & size)
{
    float buffer[] =
    {
        0.0f, 1.0f, 0.0f, size.y,
        0.0f, 0.0f, 0.0f, 0.0f,
        1.0f, 1.0f, size.x, size.y,
        1.0f, 0.0f, size.x, 0.0f
    };

    glGenVertexArrays(1,&mVertexArray);
    glBindVertexArray(mVertexArray);

    glGenBuffers(1, &mVertexBuffer);

    glBindBuffer(GL_ARRAY_BUFFER, mVertexBuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(buffer), buffer, GL_STATIC_DRAW);

    glVertexAttribPointer(Renderer::Shader::TexCoords, 2, GL_FLOAT, GL_FALSE, 4*sizeof(float), (void*)0);
    glEnableVertexAttribArray(Renderer::Shader::TexCoords);

    glVertexAttribPointer(Renderer::Shader::Position, 2, GL_FLOAT, GL_FALSE, 4*sizeof(float), (void*)(sizeof(float)*2));
    glEnableVertexAttribArray(Renderer::Shader::Position);

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

Quad::~Quad()
{
    if(mVertexArray)
    {
        glDeleteBuffers(1, &mVertexBuffer);
        glDeleteVertexArrays(1, &mVertexArray);
    }
}

Quad::Quad(Quad && other)
    : mVertexArray(other.mVertexArray)
    , mVertexBuffer(other.mVertexBuffer)
{
    other.mVertexArray = 0;
}

void Quad::Render()
{
    glBindVertexArray(mVertexArray);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    glBindVertexArray(0);
}

Buffer::Buffer(const glm::vec2 & size, unsigned components, bool doubled, bool depth)
    : RenderTarget(size.x, size.y)
    , mQuad(size)
{
    Add(size, components, depth);
    if(doubled) Add(size, components, depth);
}

Buffer & Buffer::operator=(Context context)
{
    context.Surface = &mQuad;
    Render({&context}, glm::mat4());
    return *this;
}

Renderer::Reader Buffer::Get()
{
    return {mTextures.front()};
}

void Buffer::ClearStencil()
{
    for(auto && t : mTextures) t.ClearStencil();
}

void Buffer::Linear()
{
    for(auto && t : mTextures) t.SetAntiAliasTexParameters();
}

void Buffer::ClampToEdge()
{
    for(auto && t : mTextures) t.SetClampToEdgeTexParameters();
}

void Buffer::Clear(const glm::vec4 & colour)
{
    for(auto && t : mTextures) t.Clear(colour);
}

void Buffer::Render(const Renderer::DrawablesVector & objects, const glm::mat4 & transform)
{
    mTextures.front().Render(objects, transform);
}

Buffer & Buffer::Swap()
{
    assert(mTextures.size() == 2);
    std::swap(mTextures.front(), mTextures.back());
    return *this;
}

Renderer::Texture & Buffer::Texture()
{
    return mTextures.front();
}

void Buffer::Render()
{
    mQuad.Render();
}

void Buffer::Add(const glm::vec2 & size, unsigned components, bool depth)
{
    mTextures.emplace_back(size.x, size.y,
                           components == 1 ? Renderer::Texture::PixelFormat::RF :
                           components == 2 ? Renderer::Texture::PixelFormat::RGF :
                           Renderer::Texture::PixelFormat::RGBAF,
                           depth ? Renderer::RenderTexture::DepthFormat::DEPTH24_STENCIL8 :
                           Renderer::RenderTexture::DepthFormat::NONE);
    mTextures.back().SetAliasTexParameters();
}

}
