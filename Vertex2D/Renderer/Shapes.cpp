//
//  Shapes.cpp
//  Vortex
//
//  Created by Maximilian Maldacker on 07/04/2014.
//
//

#include "Shapes.h"
#include <limits>

namespace Renderer
{

Shape::Shape()
    : mNumVertices(0)
    , mColourUniform(Program::PositionProgram(), "u_Colour")
    , mProgram(&Program::PositionProgram())
{
    glGenVertexArrays(1,&mVertexArray);
    glBindVertexArray(mVertexArray);

    glGenBuffers(1, &mVertexBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, mVertexBuffer);

    glVertexAttribPointer(Shader::Position, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);
    glEnableVertexAttribArray(Shader::Position);

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

Shape::~Shape()
{
    if(mVertexArray)
    {
        glDeleteBuffers(1, &mVertexBuffer);
        glDeleteVertexArrays(1, &mVertexArray);
    }
}

Shape::Shape(Shape && other)
    : Transformable(other)
    , Colour(other.Colour)
    , mType(other.mType)
    , mVertexBuffer(other.mVertexBuffer)
    , mVertexArray(other.mVertexArray)
    , mNumVertices(other.mNumVertices)
    , mColourUniform(other.mColourUniform)
    , mProgram(other.mProgram)
{
    other.mVertexArray = 0;
}

void Shape::SetType(GLuint type)
{
    mType = type;
}

void Shape::SetProgram(Program & program)
{
    mProgram = &program;
    mColourUniform.SetLocation(program, "u_Colour");
}

void Shape::Set(const Path & path)
{
    mNumVertices = (uint32_t)path.size();

    if(mNumVertices > 0)
    {
        glBindBuffer(GL_ARRAY_BUFFER, mVertexBuffer);
        glBufferData(GL_ARRAY_BUFFER, 2*mNumVertices*sizeof(path[0][0]), &path[0][0], GL_DYNAMIC_DRAW);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }
}

void Shape::Render(const glm::mat4 & ortho)
{
    if(mNumVertices > 0)
    {
        mProgram->Use().SetMVP(GetTransform(ortho));

        mColourUniform.Set(Colour);

        glBindVertexArray(mVertexArray);
        glDrawArrays(mType, 0, mNumVertices);
        glBindVertexArray(0);

        mProgram->Unuse();
    }
}

Rectangle::Rectangle(const glm::vec2 & size)
{
    SetRectangle(size);
}

void Rectangle::SetRectangle(const glm::vec2 &size)
{
    SetType(GL_TRIANGLES);
    Set({{0.0f, 0.0f}, {size.x, 0.0f}, {0.0f, size.y}, {size.x, 0.0f,}, {size.x, size.y}, {0.0f, size.y}});
}

int GetNumSegments(float size)
{
    return 6.0f * std::sqrt(size);
}

Path MakeCircle(float radius)
{
    int segs = GetNumSegments(radius);

    Path circle;
    const float coef = 2.0f * (float)M_PI/segs;

    circle.emplace_back(radius, radius);
    for(int i = 0;i <= segs; i++)
    {
        float rads = i*coef;
        int j = radius * cosf(rads);
        int k = radius * sinf(rads);
        circle.emplace_back(j+radius,k+radius);
    }
    
    return circle;
}

Circle::Circle(float size)
{
    SetCircle(size);
}

void Circle::SetCircle(float size)
{
    SetType(GL_TRIANGLE_FAN);
    Set(MakeCircle(size));
}

}