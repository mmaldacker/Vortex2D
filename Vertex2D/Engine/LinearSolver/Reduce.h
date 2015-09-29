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
#include "Operator.h"

namespace Fluid
{

class Reduce
{
public:
    Reduce(glm::vec2 size);

    void apply(Renderer::RenderTexture & a, Renderer::RenderTexture & b);
    float Get();
    void Bind(int n);

//private:
    std::vector<Renderer::RenderTexture> s;
    std::vector<Renderer::Quad> q;
    Operator mReduce;
    Renderer::Uniform<glm::vec2> h;
    Renderer::Uniform<glm::vec2> k;
    Operator multiply;

    std::unique_ptr<Renderer::Reader> reader;

};

}
#endif /* defined(__Vertex2D__Reduce__) */
