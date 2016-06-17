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

/**
 * @brief Used to move and render colour with the velocity field of the Engine
 */
class Density : public Renderer::Drawable, public Renderer::Transformable
{
public:
    Density(Dimensions dimensions);

    /**
     * @brief Renders some colours on the density grid
     */
    void Render(Renderer::Drawable & object);

    /**
     * @brief Advect the colours rendered with Render using the velocity field in Engine
     */
    void Advect(Engine & engine);

    /**
     * @brief Renders the colours to a RenderTarget
     */
    void Render(Renderer::RenderTarget & target, const glm::mat4 & transform) override;

private:
    Dimensions mDimensions;
    Buffer mDensity;
};

}

#endif /* defined(__Vertex2D__Density__) */
