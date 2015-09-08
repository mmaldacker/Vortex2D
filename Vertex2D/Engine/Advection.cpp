//
//  Advection.cpp
//  Vertex2D
//
//  Created by Maximilian Maldacker on 08/09/2015.
//  Copyright (c) 2015 Maximilian Maldacker. All rights reserved.
//

#include "Advection.h"
#include "Disable.h"

namespace Fluid
{

Advection::Advection(Dimensions dimensions, float dt)
    : mDimensions(dimensions)
    , mQuad(dimensions.Size)
    , mVelocity(dimensions.Size.x, dimensions.Size.y, Renderer::Texture::PixelFormat::RGF, Renderer::RenderTexture::DepthFormat::DEPTH24_STENCIL8)
    , mDensity(dimensions.Size.x, dimensions.Size.y, Renderer::Texture::PixelFormat::RGBA8888, Renderer::RenderTexture::DepthFormat::DEPTH24_STENCIL8)
    , mAdvectShader("Diff.vsh", "Advect.fsh")
    , mAdvectDensityShader("Diff.vsh", "AdvectDensity.fsh")
    , mDensitySprite(mDensity.Front)
{
    mDensity.Front.SetAntiAliasTexParameters();
    mDensity.Back.SetAntiAliasTexParameters();
    mDensitySprite.Scale = glm::vec2(dimensions.Scale);

    mVelocity.Clear();
    mDensity.Clear();

    mAdvectShader.Use()
    .Set("delta", dt)
    .Set("u_texture", 0)
    .Set("u_velocity", 1)
    .Set("h", mQuad.Size())
    .Unuse();

    mAdvectDensityShader.Use()
    .Set("delta", dt)
    .Set("u_texture", 0)
    .Set("u_velocity", 1)
    .Set("h", mQuad.Size())
    .Unuse();
}

void Advection::RenderVelocity(const std::vector<Renderer::Drawable*> & objects)
{
    Renderer::Enable e(GL_STENCIL_TEST);
    glStencilFunc(GL_EQUAL, 0, 0xFF);
    glStencilMask(0x00);

    mVelocity.begin();
    for(auto object : objects)
    {
        object->Render(mVelocity.Orth*mDimensions.InvScale);
    }
    mVelocity.end();
}

void Advection::RenderDensity(const std::vector<Renderer::Drawable*> & objects)
{
    Renderer::Enable e(GL_STENCIL_TEST);
    glStencilFunc(GL_EQUAL, 0, 0xFF);
    glStencilMask(0x00);
    
    mDensity.begin();
    for(auto object : objects)
    {
        object->Render(mDensity.Orth*mDimensions.InvScale);
    }
    mDensity.end();
}

Renderer::Reader Advection::GetVelocityReader()
{
    return {mVelocity.Front};
}

Renderer::Sprite & Advection::GetDensity()
{
    return mDensitySprite;
}

void Advection::Advect()
{
    Advect(mVelocity, mAdvectShader);
    Advect(mDensity, mAdvectDensityShader);
}

void Advection::Advect(Renderer::PingPong & renderTexture, Renderer::Program & program)
{
    Renderer::Enable e(GL_STENCIL_TEST);
    glStencilFunc(GL_EQUAL, 0, 0xFF);
    glStencilMask(0x00);

    renderTexture.swap();
    renderTexture.begin({0.0f, 0.0f, 0.0f, 0.0f});
    program.Use().SetMVP(renderTexture.Orth);

    mVelocity.Back.Bind(1);
    renderTexture.Back.Bind(0);

    mQuad.Render();
    
    program.Unuse();
    renderTexture.end();
}

}