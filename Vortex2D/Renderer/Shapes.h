//
//  Shapes.h
//  Vortex2D
//

#ifndef Vortex_Shapes_h
#define Vortex_Shapes_h

#include "Common.h"
#include "Shader.h"
#include "Drawable.h"
#include "Transformable.h"

#include <vector>

namespace Vortex2D { namespace Renderer {

typedef std::vector<glm::vec2> Path;

/**
 * @brief Generic class to render a solid coloured shape using the basic OpenGL primitives
 */
class Shape : public Drawable, public Transformable
{
public:
    Shape();
    ~Shape();

    Shape(Shape&&);

    /**
     * @brief SetType sets the primitive type of this shape
     * @param type must be GL_POINTS, GL_TRIANGLES, etc
     */
    void SetType(GLuint type);

    /**
     * @brief Set sets a list of points (e.g. triangles)
     */
    void Set(const Path& path);

    /**
     * @brief SetProgram the program to be used when rendering
     */
    void SetProgram(Program& program);

    void Render(RenderTarget& target, const glm::mat4& transform = glm::mat4()) override;

    glm::vec4 Colour;

private:
    GLuint mType;
    GLuint mVertexBuffer;
    GLuint mVertexArray;

    GLsizei mNumVertices;

    Uniform<glm::vec4> mColourUniform;
    Program* mProgram;
};

/**
 * @brief A solid colour rectangle defined by two triangles. Implements the Drawable interface and Transformable interface.
 */
struct Rectangle : Shape
{
    Rectangle() = default;
    Rectangle(const glm::vec2& size);

    /**
     * @brief Sets the rectangle size
     */
    void SetRectangle(const glm::vec2& size);
};

/**
 * @brief A solid colour ellipse. Implements the Drawable interface and Transformable interface.
 */
class Ellipse : public Shape
{
public:
    Ellipse() = default;
    Ellipse(const glm::vec2& radius);

    void SetEllipse(const glm::vec2& radius);

    void Render(RenderTarget& target, const glm::mat4& transform = glm::mat4()) override;

private:
    Program mProgram;
    glm::vec2 mRadius;
};

}}

#endif
