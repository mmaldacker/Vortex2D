//
//  Density.h
//  Vertex2D
//
//  Created by Maximilian Maldacker on 05/04/2016.
//  Copyright (c) 2016 Maximilian Maldacker. All rights reserved.
//

#ifndef __Vertex2D__Density__
#define __Vertex2D__Density__

#include "Size.h"
#include "Operator.h"
#include "Drawable.h"
#include "Transformable.h"

namespace Fluid
{

class Engine;

class Density : public Renderer::Drawable, public Renderer::Transformable
{
public:
    Density(Dimensions dimensions, float dt);

    void Render(const Renderer::DrawablesVector & objects);
    void Advect(Engine & engine);

    void Render(Renderer::RenderTarget & target, const glm::mat4 & transform) override;

private:
    Dimensions mDimensions;
    Buffer mDensity;
};

}

#endif /* defined(__Vertex2D__Density__) */
