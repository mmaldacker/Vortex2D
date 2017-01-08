//
//  Shapes.cpp
//  Vortex2D
//

#include "Shapes.h"
#include "RenderTarget.h"
#include "Disable.h"
#include <limits>

namespace Vortex2D { namespace Renderer {

namespace
{

const char * CircleVert = GLSL(
    in vec2 a_Position;

    uniform mat4 u_Projection;
    uniform float u_radius;

    void main()
    {
        gl_PointSize = u_radius;
        gl_Position = u_Projection * vec4(a_Position, 0.0, 1.0);
    }
);

const char * CircleFrag = GLSL(
    out vec4 out_color;

    uniform vec4 u_Colour;

    void main()
    {
        vec2 pos = 2.0 * (gl_PointCoord - 0.5);
        float distance = dot(pos, pos);
        float factor = 1.0 - step(1.0, distance);
        out_color = u_Colour * factor;
    }
);

}

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
    mNumVertices = (GLsizei)path.size();

    if(mNumVertices > 0)
    {
        glBindBuffer(GL_ARRAY_BUFFER, mVertexBuffer);
        glBufferData(GL_ARRAY_BUFFER, 2u*mNumVertices*sizeof(path[0][0]), &path[0][0], GL_DYNAMIC_DRAW);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }
}

void Shape::Render(RenderTarget & target, const glm::mat4 & transform)
{
    if(mNumVertices > 0)
    {
        mProgram->Use().SetMVP(target.Orth*transform*GetTransform());

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

Circle::Circle(float size)
    : mProgram(CircleVert, CircleFrag)
{
    SetProgram(mProgram);
    SetCircle(size);
}

void Circle::SetCircle(float size)
{
    SetType(GL_POINTS);
    Set({{size,size}});
    mProgram.Use().Set("u_radius", size).Unuse();
}

void Circle::Render(RenderTarget& target, const glm::mat4& transform)
{
    Enable e(GL_PROGRAM_POINT_SIZE);
    Shape::Render(target, transform);
}

}}
