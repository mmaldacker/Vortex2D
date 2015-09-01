//
//  Sprite.cpp
//  Vortex
//
//  Created by Maximilian Maldacker on 07/04/2014.
//
//

#include "Sprite.h"

#include <glm/gtc/matrix_transform.hpp>

namespace Renderer
{

Sprite::Sprite(const Texture & texture) : Sprite(texture, Quad({texture.Width(), texture.Height()}))
{

}

Sprite::Sprite(const Texture & texture, Quad && quad)
    : Colour({1,1,1,1})
    , mQuad(std::move(quad))
    , mTexture(texture)
    , mColourUniform(Program::ColourTexturePositionProgram(), "u_Colour")
{

}

Sprite::Sprite(Sprite && other)
    : Transformable(other)
    , Colour(other.Colour)
    , mQuad(std::move(other.mQuad))
    , mTexture(other.mTexture)
    , mColourUniform(other.mColourUniform)
{
}

void Sprite::Render(const glm::mat4 & ortho)
{
    Program::ColourTexturePositionProgram().Use().SetMVP(GetTransform(ortho));
    mColourUniform.Set(Colour);
    mTexture.Bind();
    mQuad.Render();
    Program::Unuse();
}

}