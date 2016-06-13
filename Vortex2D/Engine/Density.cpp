//
//  Density.cpp
//  Vertex2D
//
//  Created by Maximilian Maldacker on 05/04/2016.
//  Copyright (c) 2016 Maximilian Maldacker. All rights reserved.
//

#include "Density.h"
#include "Disable.h"
#include "Engine.h"

namespace Fluid
{

Density::Density(Dimensions dimensions, float dt)
    : mDimensions(dimensions)
    , mDensity(dimensions.Size, 4, true, true)
    , mQuad(dimensions.Size*dimensions.Scale)
{
    mDensity.clear();
}

void Density::Render(const std::vector<Renderer::Drawable*> & objects)
{
    mDensity.begin();
    for(auto object : objects)
    {
        object->Render(mDensity.Orth*mDimensions.InvScale);
    }
    mDensity.end();
}

void Density::Advect(Fluid::Engine &engine)
{
    engine.Advect(mDensity);
}

void Density::Render(const glm::mat4 &orth)
{
    Renderer::Program::TexturePositionProgram().Use().SetMVP(GetTransform(orth));
    mDensity.texture().Bind();
    mQuad.Render();
    Renderer::Texture::Unbind();
    Renderer::Program::TexturePositionProgram().Unuse();
}

}