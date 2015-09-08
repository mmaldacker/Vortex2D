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

class Boundaries
{
public:
    Boundaries(Dimensions dimensions, int antialias);

    void Render(const std::vector<Renderer::Drawable*> & objects);
    void RenderVelocities(const std::vector<Renderer::Drawable*> & objects);
    void RenderWeights();

    void BindBoundary(int n);
    void BindWeights(int n);
    void BindVelocity(int n);

    void Clear();
    Renderer::Reader GetWeightsReader();
    Renderer::Reader GetReader();

private:
    Dimensions mDimensions;
    int mAntialias;

    Renderer::Quad mQuad;
    
    Renderer::RenderTexture mBoundaries;
    Renderer::RenderTexture mBoundariesVelocity;
    Renderer::RenderTexture mWeights;

    Renderer::Program mWeightsShader;

    Renderer::Rectangle mHorizontal;
    Renderer::Rectangle mVertical;
};

}

#endif /* defined(__Vertex2D__Boundaries__) */
