//
//  Reduce.h
//  Vertex2D
//
//  Created by Maximilian Maldacker on 15/09/2015.
//  Copyright (c) 2015 Maximilian Maldacker. All rights reserved.
//

#ifndef __Vertex2D__Reduce__
#define __Vertex2D__Reduce__

#include "RenderTexture.h"
#include "Shader.h"
#include "Quad.h"
#include "Reader.h"

namespace Fluid
{

class Reduce
{
public:
    Reduce(Renderer::RenderTexture & r);

    float Get();

private:
    Renderer::RenderTexture & r;
    Renderer::Quad t;
    std::vector<Renderer::RenderTexture> s;
    std::vector<Renderer::Quad> q;
    Renderer::Program mReduceShader;
    Renderer::Uniform<glm::vec2> h;
    Renderer::Uniform<glm::vec2> k;

    std::unique_ptr<Renderer::Reader> reader;
};

}
#endif /* defined(__Vertex2D__Reduce__) */
