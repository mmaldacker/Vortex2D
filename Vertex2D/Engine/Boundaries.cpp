//
//  Boundaries.cpp
//  Vertex2D
//
//  Created by Maximilian Maldacker on 01/09/2015.
//  Copyright (c) 2015 Maximilian Maldacker. All rights reserved.
//

#include "Boundaries.h"

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
    , mHorizontal({antialias*dimensions.Size.x, antialias})
    , mVertical({antialias, antialias*dimensions.Size.y})
{
    mBoundariesVelocity.SetAliasTexParameters();

    mVertical.Colour = {1.0f,1.0f,1.0f,1.0f};
    mHorizontal.Colour = {1.0f,1.0f,1.0f,1.0f};

    mWeightsShader.Use().Set("h", mQuad.Size()).Set("u_texture", 0).Unuse();
}

void Boundaries::Render(const std::vector<Renderer::Drawable*> & objects)
{
    mBoundaries.begin({0.0f, 0.0f, 0.0f, 0.0f});
    mHorizontal.Position = {0.0f, 0.0f};
    mHorizontal.Render(mBoundaries.Orth);

    mHorizontal.Position = {0.0f, mAntialias*(mDimensions.Size.y-1.0f)};
    mHorizontal.Render(mBoundaries.Orth);

    mVertical.Position = {0.0f, 0.0f};
    mVertical.Render(mBoundaries.Orth);

    mVertical.Position = {mAntialias*(mDimensions.Size.x-1.0f), 0.0f};
    mVertical.Render(mBoundaries.Orth);

    auto scaled = glm::scale(mBoundaries.Orth, glm::vec3(mAntialias, mAntialias, 1.0f));
    for(auto object : objects)
    {
        object->Render(scaled*mDimensions.InvScale);
    }
    mBoundaries.end();
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

void Boundaries::BindBoundary(int n)
{
    mBoundaries.Bind(n);
}

void Boundaries::BindWeights(int n)
{
    mWeights.Bind(n);
}

void Boundaries::BindVelocity(int n)
{
    mBoundariesVelocity.Bind(n);
}

}