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
#include <Vortex2D/Renderer/Buffer.h>
#include <Vortex2D/Renderer/Device.h>

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

    Shape(const Device& device, Type type, const Path& path);

    void Render(const Device& device, RenderTarget & target) override;

    glm::vec4 Colour;

private:
    uint32_t mCount;
    PositionProgram mProgram;
    VertexBuffer<glm::vec2> mVertexBuffer;
    vk::UniquePipelineLayout mPipelineLayout;
    vk::PipelineInputAssemblyStateCreateInfo mInputAssembly;
    vk::UniquePipeline mPipeline;

    // TODO will need to replace with map
    std::vector<vk::CommandBuffer> mCommandBuffers;
};

/**
 * @brief A solid colour rectangle defined by two triangles. Implements the Drawable interface and Transformable interface.
 */
class Rectangle : public Shape
{
public:
    Rectangle(const Device& device, const glm::vec2& size);

};

/**
 * @brief A solid colour ellipse. Implements the Drawable interface and Transformable interface.
 */
class Ellipse : public Drawable, public Transformable
{
public:
    Ellipse(const Device& device, const glm::vec2& radius);

    void Render(const Device& device, RenderTarget & target) override;

    glm::vec4 Colour;

private:
    glm::vec2 mRadius;
};

}}

#endif
