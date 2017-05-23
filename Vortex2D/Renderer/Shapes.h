//
//  Shapes.h
//  Vortex2D
//

#ifndef Vortex_Shapes_h
#define Vortex_Shapes_h

#include <Vortex2D/Renderer/Common.h>
#include <Vortex2D/Renderer/Pipeline.h>
#include <Vortex2D/Renderer/Drawable.h>
#include <Vortex2D/Renderer/Transformable.h>
#include <Vortex2D/Renderer/Buffer.h>
#include <Vortex2D/Renderer/Device.h>
#include <Vortex2D/Renderer/DescriptorSet.h>

#include <vector>

namespace Vortex2D { namespace Renderer {

typedef std::vector<glm::vec2> Path;
class RenderTarget;

class Shape : public Drawable, public Transformable
{
public:
    Shape(const Device& device, const std::vector<glm::vec2>& vertices, const glm::vec4& colour);

    void Initialize(const RenderState& renderState) override;
    void Update(const glm::mat4& model, const glm::mat4& view) override;
    void Draw(vk::CommandBuffer commandBuffer, const RenderState& renderState) override;

private:
    vk::Device mDevice;
    Buffer mMVPBuffer;
    Buffer mColourBuffer;
    Buffer mVertexBuffer;
    DescriptorSet mDescriptorSet;
    PipelineLayout mPipelineLayout;
    GraphicsPipeline mPipeline;
    uint32_t mNumVertices;
};

/**
 * @brief A solid colour rectangle defined by two triangles. Implements the Drawable interface and Transformable interface.
 */
class Rectangle : public Shape
{
public:
    Rectangle(const Device& device, const glm::vec2& size, const glm::vec4& colour);
};

/**
 * @brief A solid colour ellipse. Implements the Drawable interface and Transformable interface.
 */
class Ellipse : public Drawable, public Transformable
{
public:
    Ellipse(const Device& device, const glm::vec2& radius);

    void Initialize(const RenderState& renderState) override;
    void Update(const glm::mat4& model, const glm::mat4& view) override;
    void Draw(vk::CommandBuffer commandBuffer, const RenderState& renderState) override;

private:
    glm::vec2 mRadius;
};

class Clear
{
public:
    Clear(uint32_t width, uint32_t height, const glm::vec4& colour);

    void Draw(vk::CommandBuffer commandBuffer);

private:
    uint32_t mWidth;
    uint32_t mHeight;
    glm::vec4 mColour;
};

}}

#endif
