//
//  Water.h
//  Vertex2D
//
//  Created by Maximilian Maldacker on 05/04/2016.
//  Copyright (c) 2016 Maximilian Maldacker. All rights reserved.
//

#ifndef __Vertex2D__Water__
#define __Vertex2D__Water__

#include "Drawable.h"
#include "Transformable.h"
#include "Size.h"
#include "Operator.h"
#include "Buffer.h"


namespace Fluid
{

class Engine;

class Water : public Renderer::Drawable, public Renderer::Transformable
{
public:
    Water(Dimensions dimensions, float dt);

    void Render(const Renderer::DrawablesVector & objects);
    void Render(Renderer::RenderTarget & target, const glm::mat4 & transform) override;
    void Redistance();
    Context GetBoundaries();
    void Advect(Engine & engine);

    glm::vec4 Colour;

private:
    Dimensions mDimensions;
    Buffer mLevelSet;
    Operator mRedistance;
    Operator mLevelSetMask;

    Renderer::Program mProgram;

    Renderer::Uniform<glm::vec4> mColourUniform;
};

}

#endif /* defined(__Vertex2D__Water__) */
