//
//  MarkerParticles.h
//  Vertex2D
//
//  Created by Maximilian Maldacker on 08/04/2016.
//  Copyright (c) 2016 Maximilian Maldacker. All rights reserved.
//

#ifndef __Vertex2D__MarkerParticles__
#define __Vertex2D__MarkerParticles__

#include "Shader.h"
#include "Shapes.h"
#include "Advection.h"

namespace Fluid
{

class MarkerParticles : public Renderer::Drawable, public Renderer::Transformable
{
public:
    MarkerParticles(float dt);
    ~MarkerParticles();

    void Set(const Renderer::Path & path);
    void Advect(Advection & advection);
    void Render(const glm::mat4 & ortho);

    glm::vec4 Colour;

private:
    Renderer::Program mAdvectionProgram;

    Renderer::Uniform<glm::vec4> mColourUniform;
    uint32_t mNumVertices;

    GLuint mVertexBuffer[2];
    GLuint mVertexArray[2];
};

}

#endif /* defined(__Vertex2D__MarkerParticles__) */
