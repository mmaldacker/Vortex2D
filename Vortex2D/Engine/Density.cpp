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

Density::Density(Dimensions dimensions)
    : mDimensions(dimensions)
    , mDensity(dimensions.Size, 4, true, true)
{
    mDensity.Linear();
    mDensity.Clear(glm::vec4(0.0));
}

void Density::Render(Renderer::Drawable & object)
{
    Renderer::Disable d(GL_BLEND);
    mDensity.Render(object, mDimensions.InvScale);
}

void Density::Advect(Engine & engine)
{
    engine.Advect(mDensity);
}

void Density::Render(Renderer::RenderTarget & target, const glm::mat4 & transform)
{
    auto & densitySprite = mDensity.Sprite();
    densitySprite.SetProgram(Renderer::Program::TexturePositionProgram());
    densitySprite.Render(target, glm::scale(glm::vec3(mDimensions.Scale, mDimensions.Scale, 1.0))*transform);
}

}