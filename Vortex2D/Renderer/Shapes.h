//
//  Shapes.h
//  Vortex
//
//  Created by Maximilian Maldacker on 07/04/2014.
//
//

#ifndef __Vortex__Shapes__
#define __Vortex__Shapes__

#include "Common.h"
#include "Shader.h"
#include "Drawable.h"
#include "Transformable.h"

#include <vector>

namespace Renderer
{

typedef std::vector<glm::vec2> Path;

class Shape : public Drawable, public Transformable
{
public:
    Shape();
    ~Shape();

    Shape(Shape &&);

    void SetType(GLuint type);
    void Set(const Path & path);
    void SetProgram(Program & program);

    void Render(RenderTarget & target, const glm::mat4 & transform) override;

    glm::vec4 Colour;

private:
    GLuint mType;
    GLuint mVertexBuffer;
    GLuint mVertexArray;

    uint32_t mNumVertices;

    Uniform<glm::vec4> mColourUniform;
    Program * mProgram;
};

struct Rectangle : Shape
{
    Rectangle() = default;
    Rectangle(const glm::vec2 & size);

    void SetRectangle(const glm::vec2 & size);
};

struct Circle : Shape
{
    Circle() = default;
    Circle(float size);

    void SetCircle(float size);
};

}

#endif /* defined(__Vortex__Shapes__) */
