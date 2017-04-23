//
//  Shapes.h
//  Vortex2D
//

#ifndef Vortex_Shapes_h
#define Vortex_Shapes_h

#include <Vortex2D/Renderer/Common.h>
#include <Vortex2D/Renderer/Shader.h>
#include <Vortex2D/Renderer/Drawable.h>
#include <Vortex2D/Renderer/Transformable.h>

#include <vector>

namespace Vortex2D { namespace Renderer {

typedef std::vector<glm::vec2> Path;

/**
 * @brief Generic class to render a solid coloured shape using the basic OpenGL primitives
 */
class Shape : public Drawable, public Transformable
{
public:
    enum class Type
    {
        TRIANGLES,
        POINTS
    };

    Shape();
    ~Shape();

    Shape(Shape&&);

    /**
     * @brief SetType sets the primitive type of this shape
     * @param type must be GL_POINTS, GL_TRIANGLES, etc
     */
    void SetType(Type type, const Path& path);

    void Render(RenderTarget& target, const glm::mat4& transform = glm::mat4()) override;

    glm::vec4 Colour;
protected:
    Shape(const char* vert, const char* frag);

    Program mProgram;

private:
    Uniform<glm::vec4> mColourUniform;
    Type mType;
};

/**
 * @brief A solid colour rectangle defined by two triangles. Implements the Drawable interface and Transformable interface.
 */
class Rectangle : public Shape
{
public:
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
    glm::vec2 mRadius;
};

}}

#endif
