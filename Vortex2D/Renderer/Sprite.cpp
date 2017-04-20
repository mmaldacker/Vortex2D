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
}

Sprite::~Sprite()
{

}

Sprite::Sprite(Sprite && other)
    : mTexture(other.mTexture)
    , mProgram(other.mProgram)
{
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
}

}}
