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
    , mDirichletBoundaries(dimensions.Size, 1)
    , mNeumannBoundaries(glm::vec2(antialias)*dimensions.Size, 1)
    , mBoundariesVelocity(dimensions.Size, 2)
    , mWeights("TexturePosition.vsh", "Weights.fsh")
    , mDiagonals("TexturePosition.vsh", "Diagonals.fsh")
    , mBoundaryMask("TexturePosition.vsh", "BoundaryMask.fsh")
    , mHorizontal({dimensions.Size.x, 1.0f})
    , mVertical({1.0f, dimensions.Size.y})
{
    mDirichletBoundaries.clear();

    mNeumannBoundaries.clear();
    mNeumannBoundaries.linear();

    mBoundariesVelocity.clear();

    mVertical.Colour = {1.0f,1.0f,1.0f,1.0f};
    mHorizontal.Colour = {1.0f,1.0f,1.0f,1.0f};

    mWeights.Use().Set("u_dirichlet", 0).Set("u_neumann", 1).Unuse();
    mDiagonals.Use().Set("u_texture", 0).Unuse();
    mBoundaryMask.Use().Set("u_dirichlet", 0).Set("u_neumann", 1).Unuse();
}

void Boundaries::RenderDirichlet(const std::vector<Renderer::Drawable*> & objects)
{
    mDirichletBoundaries.begin();
    for(auto object : objects)
    {
        object->Render(mNeumannBoundaries.Orth*mDimensions.InvScale);
    }
    mDirichletBoundaries.end();
}

void Boundaries::RenderNeumann(const std::vector<Renderer::Drawable*> & objects)
{
    mNeumannBoundaries.begin();
    auto scaled = glm::scale(mNeumannBoundaries.Orth, glm::vec3(mAntialias, mAntialias, 1.0f));
    for(auto object : objects)
    {
        object->Render(scaled*mDimensions.InvScale);
    }
    mNeumannBoundaries.end();
}

void Boundaries::RenderMask(Buffer & mask)
{
    Renderer::Enable e(GL_STENCIL_TEST);
    Renderer::DisableColorMask c;

    glStencilFunc(GL_ALWAYS, 1, 0xFF); // write 1 in stencil buffer
    glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE); // replace value with above
    glStencilMask(0xFF); // enable stencil writing

    // clear stencil buffer
    mask.begin();
    glClearStencil(0);
    glClear(GL_STENCIL_BUFFER_BIT);
    mask.end();

    mask = mBoundaryMask(mDirichletBoundaries, mNeumannBoundaries);

    glStencilMask(0x00); // disable stencil writing
}

void Boundaries::RenderBorders()
{
    mNeumannBoundaries.begin();

    mHorizontal.Position = {0.0f, 0.0f};
    mHorizontal.Scale = {mAntialias, mAntialias};
    mHorizontal.Render(mNeumannBoundaries.Orth);

    mHorizontal.Position = {0.0f, mAntialias*mDimensions.Size.y-mAntialias};
    mHorizontal.Render(mNeumannBoundaries.Orth);

    mVertical.Position = {0.0f, 0.0f};
    mVertical.Scale = {mAntialias, mAntialias};
    mVertical.Render(mNeumannBoundaries.Orth);

    mVertical.Position = {mAntialias*mDimensions.Size.x-mAntialias, 0.0f};
    mVertical.Render(mNeumannBoundaries.Orth);

    mNeumannBoundaries.end();
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
    
void Boundaries::RenderFluid(Fluid::MarkerParticles &markerParticles)
{
    auto colour = markerParticles.Colour;
    mDirichletBoundaries.begin({1.0f, 0.0f, 0.0f, 0.0f});
    markerParticles.Colour = glm::vec4{0.0f};
    markerParticles.Render(mDirichletBoundaries.Orth*mDimensions.InvScale);
    mDirichletBoundaries.end();
    markerParticles.Colour = colour;
}

void Boundaries::Clear()
{
    mDirichletBoundaries.clear();
    mNeumannBoundaries.clear();
    mBoundariesVelocity.clear();
}

}