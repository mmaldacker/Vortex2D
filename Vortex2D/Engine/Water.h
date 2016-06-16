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

/**
 * @brief Class to simulate water using the LevelSet method. This contains a full size grid
 * with sections marked as water and others marked as air.
 */
class Water : public Renderer::Drawable, public Renderer::Transformable
{
public:
    Water(Dimensions dimensions, float dt);

    /**
     * @brief Renders water section.
     * @param object
     */
    void Render(Renderer::Drawable & object);

    /**
     * @brief Renders the water on a RenderTarget
     * @param target
     * @param transform
     */
    void Render(Renderer::RenderTarget & target, const glm::mat4 & transform) override;

    /**
     * @brief Updates the LevelSet to more accurately represent distance to water sections.
     */
    void Redistance();

    /**
     * @brief Gets the air section of the water. Has to be used to mark dirichlet boundaries in the engine.
     * @return returns a Sprite used to draw the dirichlet boundaries.
     */
    Renderer::Sprite & GetBoundaries();

    /**
     * @brief Advects the LevelSet
     * @param engine
     */
    void Advect(Engine & engine);

    /**
     * @brief Colour of the water. The air is transparent.
     */
    glm::vec4 Colour;

private:
    Dimensions mDimensions;
    Buffer mLevelSet;
    Operator mRedistance;
    
    Renderer::Program mLevelSetMask;
    Renderer::Program mProgram;

    Renderer::Uniform<glm::vec4> mColourUniform;
};

}

#endif /* defined(__Vertex2D__Water__) */
