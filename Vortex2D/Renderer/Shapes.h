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

// TODO all buffer are host local, not the most performant...
class AbstractShape : public Drawable, public Transformable
{
public:
    // TODO have colour has member variable and updated in the Update function
    // do this also for Ellipse
    AbstractShape(const Device& device,
                  const SpirvBinary& fragShader,
                  const std::vector<glm::vec2>& vertices,
                  const glm::vec4& colour);
    AbstractShape(AbstractShape&& other);
    virtual ~AbstractShape() {}

    void Initialize(const RenderState& renderState) override;
    void Update(const glm::mat4& projection, const glm::mat4& view) override;
    void Draw(vk::CommandBuffer commandBuffer, const RenderState& renderState) override;

protected:
    const Device& mDevice;
    UniformBuffer<glm::mat4> mMVPBuffer;
    UniformBuffer<glm::vec4> mColourBuffer;
    VertexBuffer<glm::vec2> mVertexBuffer;
    DescriptorSet mDescriptorSet;
    GraphicsPipeline mPipeline;
    uint32_t mNumVertices;
};

/**
 * @brief A solid colour rectangle defined by two triangles. Implements the Drawable interface and Transformable interface.
 */
class Rectangle : public AbstractShape
{
public:
    Rectangle(const Device& device, const glm::vec2& size, const glm::vec4& colour);
};

class IntRectangle : public AbstractShape
{
public:
    IntRectangle(const Device& device, const glm::vec2& size, const glm::ivec4& colour);

    void Update(const glm::mat4& projection, const glm::mat4& view) override;
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

    const Device& mDevice;
    glm::vec2 mRadius;
    UniformBuffer<glm::mat4> mMVPBuffer;
    UniformBuffer<glm::vec4> mColourBuffer;
    VertexBuffer<glm::vec2> mVertexBuffer;
    UniformBuffer<Size> mSizeBuffer;
    DescriptorSet mDescriptorSet;
    GraphicsPipeline mPipeline;
};

class Clear : public Drawable
{
public:
    Clear(const glm::vec4& colour);

    void Initialize(const RenderState& renderState) override;
    void Update(const glm::mat4& projection, const glm::mat4& view) override;
    void Draw(vk::CommandBuffer commandBuffer, const RenderState& renderState) override;

private:
    glm::vec4 mColour;
};

}}

#endif
