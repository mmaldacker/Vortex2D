//
//  Shapes.cpp
//  Vortex2D
//

#include "Shapes.h"

#include <Vortex2D/Renderer/RenderTarget.h>
#include <Vortex2D/Renderer/Disable.h>

#include <limits>
#include <algorithm>

namespace Vortex2D { namespace Renderer {

namespace
{

const char* EllipseVert = GLSL(
    in vec2 a_Position;

    uniform mat4 u_Projection;
    uniform float u_size;
    uniform vec2 u_radius;

    void main()
    {
        gl_PointSize = 2 * u_size + 1;
        gl_Position = u_Projection * vec4(a_Position, 0.0, 1.0);
    }
);

const char* EllipseFrag = GLSL(
    out vec4 out_color;

    uniform vec4 u_Colour;
    uniform float u_size;
    uniform vec2 u_radius;
    uniform mat2 u_rotation;

    void main()
    {
        float size = 2 * u_size + 1;
        vec2 pos = (gl_PointCoord * size) - u_size;
        pos = u_rotation * pos;
        float distance = dot(pos / u_radius, pos / u_radius);
        // gl_PointCoord is not exactly 0.0 on the left/top borders
        // thus we need to include a tolerance so the left/top points are drawn
        if (distance - 1.0 <= 1e-5)
        {
            out_color = u_Colour;
        }
        else
        {
            discard;
        }
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
    if (mVertexArray)
    {
        glDeleteBuffers(1, &mVertexBuffer);
        glDeleteVertexArrays(1, &mVertexArray);
    }
}

Shape::Shape(Shape&& other)
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

void Shape::SetProgram(Program& program)
{
    mProgram = &program;
    mColourUniform.SetLocation(program, "u_Colour");
}

void Shape::Set(const Path& path)
{
    mNumVertices = (GLsizei)path.size();

    if (mNumVertices > 0)
    {
        glBindBuffer(GL_ARRAY_BUFFER, mVertexBuffer);
        glBufferData(GL_ARRAY_BUFFER, 2u * mNumVertices * sizeof(path[0][0]), &path[0][0], GL_DYNAMIC_DRAW);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }
}

void Shape::Render(RenderTarget& target, const glm::mat4& transform)
{
    if (mNumVertices > 0)
    {
        mProgram->Use().SetMVP(target.Orth * transform*GetTransform());

        mColourUniform.Set(Colour);

        glBindVertexArray(mVertexArray);
        glDrawArrays(mType, 0, mNumVertices);
        glBindVertexArray(0);

        mProgram->Unuse();
    }
}

Rectangle::Rectangle(const glm::vec2& size)
{
    SetRectangle(size);
}

void Rectangle::SetRectangle(const glm::vec2& size)
{
    SetType(GL_TRIANGLES);
    Set({{0.0f, 0.0f}, {size.x, 0.0f}, {0.0f, size.y}, {size.x, 0.0f,}, {size.x, size.y}, {0.0f, size.y}});
}

Ellipse::Ellipse(const glm::vec2& radius)
    : mProgram(EllipseVert, EllipseFrag)
{
    SetProgram(mProgram);
    SetEllipse(radius);
}

void Ellipse::SetEllipse(const glm::vec2& radius)
{
    SetType(GL_POINTS);
    Set({{0.0f,0.0f}});
    mRadius = radius;
}

void Ellipse::Render(RenderTarget& target, const glm::mat4& transform)
{
    Enable e(GL_PROGRAM_POINT_SIZE);
    EnableParameter ep(glPointParameteri, GL_POINT_SPRITE_COORD_ORIGIN, GL_LOWER_LEFT);

    glm::vec2 transformScale(glm::length(transform[0]), glm::length(transform[1]));
    glm::vec2 radius = mRadius * (glm::vec2)Scale * transformScale;
    glm::mat4 rotation4 = glm::rotate(glm::radians((float)Rotation), glm::vec3(0.0f, 0.0f, 1.0f));
    glm::mat2 rotation(rotation4[0].xy, rotation4[1].xy);
    mProgram.Use()
            .Set("u_radius", radius)
            .Set("u_size", std::max(radius.x, radius.y))
            .Set("u_rotation", rotation);
    Shape::Render(target, transform);
}

}}
