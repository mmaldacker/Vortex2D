//
//  Sprite.cpp
//  Vortex2D
//

#include "Sprite.h"

#include <Vortex2D/Renderer/RenderTarget.h>

namespace Vortex2D { namespace Renderer {

Sprite::Sprite(const glm::vec2 & size) : mTexture(nullptr), mProgram(nullptr)
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

Sprite::~Sprite()
{
    if(mVertexArray)
    {
        glDeleteBuffers(1, &mVertexBuffer);
        glDeleteVertexArrays(1, &mVertexArray);
    }
}

Sprite::Sprite(Sprite && other)
    : mVertexArray(other.mVertexArray)
    , mVertexBuffer(other.mVertexBuffer)
    , mTexture(other.mTexture)
    , mProgram(other.mProgram)
{
    other.mVertexArray = 0;
    other.mTexture = nullptr;
    other.mProgram = nullptr;
}

void Sprite::SetTexture(Texture & texture)
{
    mTexture = &texture;
}

void Sprite::NoTexture()
{
    mTexture = nullptr;
}

void Sprite::SetProgram(Program & program)
{
    mProgram = &program;
}

void Sprite::Render(RenderTarget & target, const glm::mat4 & transform)
{
    assert(mProgram);

    mProgram->Use().SetMVP(target.Orth*transform);
    if(mTexture) mTexture->Bind();

    glBindVertexArray(mVertexArray);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    glBindVertexArray(0);

    Renderer::Texture::Unbind();
    mProgram->Unuse();
}

}}
