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
#include "LevelSet.h"

namespace Fluid
{

class Water : public Renderer::Drawable, public Renderer::Transformable
{
public:
    Water(Fluid::LevelSet & levelSet);

    void Render(const glm::mat4 & ortho) override;

private:
    Renderer::Program mProgram;
    Renderer::Texture & mWaterTexture;
    Renderer::Quad mQuad;
};

}

#endif /* defined(__Vertex2D__Water__) */
