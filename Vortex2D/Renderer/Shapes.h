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

/**
 * @brief An polygonal shape where the fragment shader can be specified for customisation.
 */
class AbstractShape : public Drawable, public Transformable
{
public:
    VORTEX2D_API AbstractShape(const Device& device,
                               const SpirvBinary& fragShader,
                               const std::vector<glm::vec2>& vertices);
    VORTEX2D_API AbstractShape(AbstractShape&& other);
    VORTEX2D_API virtual ~AbstractShape() = default;

    VORTEX2D_API void Initialize(const RenderState& renderState) override;
    VORTEX2D_API void Update(const glm::mat4& projection, const glm::mat4& view) override;
    VORTEX2D_API void Draw(vk::CommandBuffer commandBuffer, const RenderState& renderState) override;

    glm::vec4 Colour;

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
    VORTEX2D_API Rectangle(const Device& device, const glm::vec2& size);
};

/**
 * @brief A solid colour rectangle as \ref Rectangle, however uses integer colors and is meant
 * to be drawn to a framebuffer wiht integer colours.
 */
class IntRectangle : public AbstractShape
{
public:
    VORTEX2D_API IntRectangle(const Device& device, const glm::vec2& size);
};

/**
 * @brief A solid colour ellipse. Implements the Drawable interface and Transformable interface.
 */
class Ellipse : public Drawable, public Transformable
{
public:
    VORTEX2D_API Ellipse(const Device& device, const glm::vec2& radius);

    void Initialize(const RenderState& renderState) override;
    void Update(const glm::mat4& projection, const glm::mat4& view) override;
    void Draw(vk::CommandBuffer commandBuffer, const RenderState& renderState) override;

    glm::vec4 Colour;

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

/**
 * @brief A drawable that simply clears the target.
 */
class Clear : public Drawable
{
public:
    VORTEX2D_API Clear(const glm::vec4& colour);

    void Initialize(const RenderState& renderState) override;
    void Update(const glm::mat4& projection, const glm::mat4& view) override;
    void Draw(vk::CommandBuffer commandBuffer, const RenderState& renderState) override;

private:
    glm::vec4 mColour;
};

}}

#endif
