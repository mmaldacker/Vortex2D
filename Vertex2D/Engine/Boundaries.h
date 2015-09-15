//
//  Boundaries.h
//  Vertex2D
//
//  Created by Maximilian Maldacker on 01/09/2015.
//  Copyright (c) 2015 Maximilian Maldacker. All rights reserved.
//

#ifndef __Vertex2D__Boundaries__
#define __Vertex2D__Boundaries__

#include "RenderTexture.h"
#include "Shapes.h"
#include "Size.h"
#include "Reader.h"

namespace Fluid
{

class Engine;

class Boundaries
{
public:
    Boundaries(Dimensions dimensions, int antialias);

    void Render(const std::vector<Renderer::Drawable*> & objects);
    void RenderVelocities(const std::vector<Renderer::Drawable*> & objects);
    void RenderMask(Renderer::RenderTexture & mask);
    void RenderWeights(Renderer::RenderTexture & w, Renderer::Quad & quad);

    void Clear();
    
    Renderer::Reader GetReader();

//private:
    void Render(const glm::mat4 & orth, int thickness = 1, float scale = 1.0f);
    
    Dimensions mDimensions;
    int mAntialias;

    Renderer::Quad mQuad;
    
    Renderer::RenderTexture mBoundaries;
    Renderer::RenderTexture mBoundariesVelocity;

    Renderer::Program mWeightsShader;

    Renderer::Rectangle mHorizontal;
    Renderer::Rectangle mVertical;

    std::vector<Renderer::Drawable*> mObjects;
};

}

#endif /* defined(__Vertex2D__Boundaries__) */
