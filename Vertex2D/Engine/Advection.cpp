//
//  Advection.cpp
//  Vertex2D
//
//  Created by Maximilian Maldacker on 08/09/2015.
//  Copyright (c) 2015 Maximilian Maldacker. All rights reserved.
//

#include "Advection.h"

namespace Fluid
{

Advection::Advection(Dimensions dimensions, Boundaries & boundaries, float dt)
    : mDimensions(dimensions)
    , mBoundaries(boundaries)
    , mQuad(dimensions.Size)
    , mVelocity(dimensions.Size.x, dimensions.Size.y, Renderer::Texture::PixelFormat::RGF)
    , mDensity(dimensions.Size.x, dimensions.Size.y, Renderer::Texture::PixelFormat::RGBA8888)
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
    .Set("xy_min", glm::vec2{0.5f, 0.5f})
    .Set("xy_max", dimensions.Size - glm::vec2{1.5})
    .Set("u_texture", 0)
    .Set("u_velocity", 1)
    .Set("u_obstacles", 2)
    .Set("h", mQuad.Size())
    .Unuse();

    mAdvectDensityShader.Use()
    .Set("delta", dt)
    .Set("xy_min", glm::vec2{0.5f, 0.5f})
    .Set("xy_max", dimensions.Size - glm::vec2{1.5})
    .Set("u_texture", 0)
    .Set("u_velocity", 1)
    .Set("u_obstacles", 2)
    .Set("h", mQuad.Size())
    .Unuse();
}

void Advection::RenderVelocity(const std::vector<Renderer::Drawable*> & objects)
{
    mVelocity.begin();
    for(auto object : objects)
    {
        object->Render(mVelocity.Orth*mDimensions.InvScale);
    }
    mVelocity.end();
}

void Advection::RenderDensity(const std::vector<Renderer::Drawable*> & objects)
{
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
    renderTexture.swap();
    renderTexture.begin({0.0f, 0.0f, 0.0f, 0.0f});
    program.Use().SetMVP(renderTexture.Orth);

    mBoundaries.BindBoundary(2);
    mVelocity.Back.Bind(1);
    renderTexture.Back.Bind(0);

    mQuad.Render();
    
    program.Unuse();
    renderTexture.end();
}

}