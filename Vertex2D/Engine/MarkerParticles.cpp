//
//  MarkerParticles.cpp
//  Vertex2D
//
//  Created by Maximilian Maldacker on 08/04/2016.
//  Copyright (c) 2016 Maximilian Maldacker. All rights reserved.
//

#include "MarkerParticles.h"
#include "Disable.h"

namespace Fluid
{

MarkerParticles::MarkerParticles(float dt)
    : mColourUniform(Renderer::Program::PositionProgram(), "u_Colour")
    , mNumVertices(0)
{
    Renderer::VertexShader advectionShader;
    advectionShader.Source("ParticleAdvection.vsh").Compile();
    mAdvectionProgram.AttachShader(advectionShader);
    mAdvectionProgram.AttachFeedback({"outPosition"});
    mAdvectionProgram.Link();

    mAdvectionProgram.Use().Set("u_velocity", 0).Set("delta", dt).Unuse();

    glGenBuffers(2, mVertexBuffer);
    glGenVertexArrays(2, mVertexArray);

    glBindVertexArray(mVertexArray[0]);
    glBindBuffer(GL_ARRAY_BUFFER, mVertexBuffer[0]);

    glVertexAttribPointer(Renderer::Shader::Position, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);
    glEnableVertexAttribArray(Renderer::Shader::Position);

    glBindVertexArray(mVertexArray[1]);
    glBindBuffer(GL_ARRAY_BUFFER, mVertexBuffer[1]);

    glVertexAttribPointer(Renderer::Shader::Position, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);
    glEnableVertexAttribArray(Renderer::Shader::Position);

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

MarkerParticles::~MarkerParticles()
{
    if(mVertexArray[0])
    {
        glDeleteBuffers(2, mVertexBuffer);
        glDeleteVertexArrays(2, mVertexArray);
    }
}

void MarkerParticles::Set(const Renderer::Path & path)
{
    mNumVertices = (uint32_t)path.size();

    if(mNumVertices > 0)
    {
        glBindBuffer(GL_ARRAY_BUFFER, mVertexBuffer[0]);
        glBufferData(GL_ARRAY_BUFFER, 2*mNumVertices*sizeof(path[0][0]), &path[0][0], GL_DYNAMIC_DRAW);

        glBindBuffer(GL_ARRAY_BUFFER, mVertexBuffer[1]);
        glBufferData(GL_ARRAY_BUFFER, 2*mNumVertices*sizeof(path[0][0]), &path[0][0], GL_STREAM_DRAW);

        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }
}

void MarkerParticles::Advect(Advection & advection)
{
    if(mNumVertices > 0)
    {
        Renderer::Enable d(GL_RASTERIZER_DISCARD);

        mAdvectionProgram.Use();

        glBindVertexArray(mVertexArray[0]);

        glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 0, mVertexBuffer[1]);

        glBeginTransformFeedback(GL_POINTS);
        advection.mVelocity.texture().Bind();
        glDrawArrays(GL_POINTS, 0, mNumVertices);
        glEndTransformFeedback();
        glFlush();

        glBindVertexArray(0);

        mAdvectionProgram.Unuse();

        std::swap(mVertexBuffer[0], mVertexBuffer[1]);
        std::swap(mVertexArray[0], mVertexArray[1]);
    }
}

void MarkerParticles::Render(const glm::mat4 & ortho)
{
    if(mNumVertices > 0)
    {
        Renderer::Program::PositionProgram().Use().SetMVP(GetTransform(ortho));

        mColourUniform.Set(Colour);

        glBindVertexArray(mVertexArray[0]);
        glDrawArrays(GL_POINTS, 0, mNumVertices);
        glBindVertexArray(0);

        Renderer::Program::PositionProgram().Unuse();
    }
}

}