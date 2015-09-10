//
//  Boundaries.cpp
//  Vertex2D
//
//  Created by Maximilian Maldacker on 01/09/2015.
//  Copyright (c) 2015 Maximilian Maldacker. All rights reserved.
//

#include "Boundaries.h"
#include "Disable.h"

namespace Fluid
{

Boundaries::Boundaries(Dimensions dimensions, int antialias)
    : mDimensions(dimensions)
    , mAntialias(antialias)
    , mQuad(dimensions.Size)
    , mBoundaries(antialias*dimensions.Size.x, antialias*dimensions.Size.y, Renderer::Texture::PixelFormat::RF)
    , mBoundariesVelocity(dimensions.Size.x, dimensions.Size.y, Renderer::Texture::PixelFormat::RGF)
    , mWeights(dimensions.Size.x, dimensions.Size.y, Renderer::Texture::PixelFormat::RGBAF)
    , mWeightsShader("Diff.vsh", "Weights.fsh")
    , mHorizontal({dimensions.Size.x, 1.0f})
    , mVertical({1.0f, dimensions.Size.y})
{
    mWeights.SetAliasTexParameters();
    mBoundariesVelocity.SetAliasTexParameters();

    mBoundaries.Clear();
    mBoundariesVelocity.Clear();
    mWeights.Clear();

    mVertical.Colour = {1.0f,1.0f,1.0f,1.0f};
    mHorizontal.Colour = {1.0f,1.0f,1.0f,1.0f};

    mWeightsShader.Use().Set("h", mQuad.Size()).Set("u_texture", 0).Unuse();
}

void Boundaries::Render(const std::vector<Renderer::Drawable*> & objects)
{
    mBoundaries.begin({0.0f, 0.0f, 0.0f, 0.0f});
    RenderAtScale(mAntialias, objects, mBoundaries.Orth);
    mBoundaries.end();
}

void Boundaries::RenderMask(Renderer::RenderTexture & mask, const std::vector<Renderer::Drawable*> & objects)
{
    Renderer::Enable e(GL_STENCIL_TEST);

    glStencilFunc(GL_ALWAYS, 1, 0xFF); // write 1 in stencil buffer
    glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE); // replace value with above
    glStencilMask(0xFF); // enable stencil writing
    glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);

    mask.begin();

    glClearStencil(0);
    glClear(GL_STENCIL_BUFFER_BIT); // clear stencil buffer

    RenderAtScale(1, objects, mask.Orth);
    mask.end();

    glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
    glStencilMask(0x00); // disable stencil writing
}

void Boundaries::RenderAtScale(int scale, const std::vector<Renderer::Drawable*> & objects, const glm::mat4 & orth)
{
    mHorizontal.Position = {0.0f, 0.0f};
    mHorizontal.Scale = glm::vec2{scale};
    mHorizontal.Render(orth);

    mHorizontal.Position = {0.0f, scale*(mDimensions.Size.y-1.0f)};
    mHorizontal.Render(orth);

    mVertical.Position = {0.0f, 0.0f};
    mVertical.Scale = glm::vec2{scale};
    mVertical.Render(orth);

    mVertical.Position = {scale*(mDimensions.Size.x-1.0f), 0.0f};
    mVertical.Render(orth);

    auto scaled = glm::scale(orth, glm::vec3(scale, scale, 1.0f));
    for(auto object : objects)
    {
        object->Render(scaled*mDimensions.InvScale);
    }
}

void Boundaries::RenderVelocities(const std::vector<Renderer::Drawable*> & objects)
{
    mBoundariesVelocity.begin({0.0f, 0.0f, 0.0f, 0.0f});
    for(auto object : objects)
    {
        object->Render(mBoundariesVelocity.Orth*mDimensions.InvScale);
    }
    mBoundariesVelocity.end();
}

void Boundaries::RenderWeights()
{
    mWeights.begin({0.0f, 0.0f, 0.0f, 0.0f});
    mWeightsShader.Use().SetMVP(mWeights.Orth);

    mBoundaries.Bind();
    mQuad.Render();

    mWeightsShader.Unuse();
    mWeights.end();
}

void Boundaries::Clear()
{
    mBoundaries.Clear();
    mBoundariesVelocity.Clear();
    mWeights.Clear();
}

Renderer::Reader Boundaries::GetReader()
{
    return {mBoundaries};
}

Renderer::Reader Boundaries::GetWeightsReader()
{
    return {mWeights};
}

}