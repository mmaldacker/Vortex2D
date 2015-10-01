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
#include "Operator.h"

namespace Fluid
{

class Boundaries
{
public:
    Boundaries(Dimensions dimensions, int antialias);

    void Render(const std::vector<Renderer::Drawable*> & objects);
    void RenderVelocities(const std::vector<Renderer::Drawable*> & objects);
    void RenderMask(Buffer & mask);
    auto GetWeights() -> decltype(std::declval<Operator>()(std::declval<Buffer&>()))
    {
        return mWeights(mBoundaries);
    }

    void Clear();

    friend class Engine;
//private:
    void Render(const glm::mat4 & orth, int thickness = 1, float scale = 1.0f);
    
    Dimensions mDimensions;
    int mAntialias;
    
    Buffer mBoundaries;
    Buffer mBoundariesVelocity;

    Operator mWeights;

    Renderer::Rectangle mHorizontal;
    Renderer::Rectangle mVertical;

    std::vector<Renderer::Drawable*> mObjects;
};

}

#endif /* defined(__Vertex2D__Boundaries__) */
