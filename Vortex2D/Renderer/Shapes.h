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
struct RenderTarget;

class Shape : public Drawable, public Transformable
{
public:
    // TODO have colour has member variable and updated in the Update function
    // do this also for Ellipse
    Shape(const Device& device, const std::vector<glm::vec2>& vertices, const glm::vec4& colour);

    void Initialize(const RenderState& renderState) override;
    void Update(const glm::mat4& projection, const glm::mat4& view) override;
    void Draw(vk::CommandBuffer commandBuffer, const RenderState& renderState) override;

private:
    vk::Device mDevice;
    Buffer mMVPBuffer;
    Buffer mColourBuffer;
    Buffer mVertexBuffer;
    vk::UniqueDescriptorSet mDescriptorSet;
    vk::UniquePipelineLayout mPipelineLayout;
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
    Ellipse(const Device& device, const glm::vec2& radius, const glm::vec4& colour);

    void Initialize(const RenderState& renderState) override;
    void Update(const glm::mat4& projection, const glm::mat4& view) override;
    void Draw(vk::CommandBuffer commandBuffer, const RenderState& renderState) override;

private:
    // std140 aligned structure
    struct Size
    {
        alignas(8) glm::vec2 view;
        alignas(8) glm::vec2 radius;
        // matrices are represented as arrays of columns, each of size 16
        alignas(16) glm::mat2x4 rotation;
    };

    vk::Device mDevice;
    glm::vec2 mRadius;
    Buffer mMVPBuffer;
    Buffer mColourBuffer;
    Buffer mVertexBuffer;
    Buffer mSizeBuffer;
    vk::UniqueDescriptorSet mDescriptorSet;
    vk::UniquePipelineLayout mPipelineLayout;
    GraphicsPipeline mPipeline;
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
