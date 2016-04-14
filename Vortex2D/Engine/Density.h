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

class Advection;
class Boundaries;

class Density : public Renderer::Drawable, public Renderer::Transformable
{
public:
    Density(Dimensions dimensions, float dt);

    void Render(const std::vector<Renderer::Drawable*> & objects);
    void Advect(Advection & advection);

    void Render(const glm::mat4 & orth) override;

private:
    Dimensions mDimensions;
    Buffer mDensity;
    Operator mAdvectDensity;
    Renderer::Quad mQuad;
};

}

#endif /* defined(__Vertex2D__Density__) */
