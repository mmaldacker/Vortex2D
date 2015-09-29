//
//  Sprite.cpp
//  Vertex2D
//
//  Created by Maximilian Maldacker on 27/09/2015.
//  Copyright (c) 2015 Maximilian Maldacker. All rights reserved.
//

#include "Sprite.h"
#include "Shader.h"

namespace Renderer
{

Sprite::Sprite(Texture & texture) : mTexture(texture), mQuad({texture.Width(), texture.Height()})
{

}


void Sprite::Render(const glm::mat4 &ortho)
{
    Program::TexturePositionProgram().Use().SetMVP(GetTransform(ortho));
    mTexture.Bind(0);
    mQuad.Render();
    mTexture.Unbind();
    Program::TexturePositionProgram().Unuse();
}

}