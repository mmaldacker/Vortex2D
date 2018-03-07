//
//  Boundaries.h
//  Vortex2D
//

#ifndef Vortex2d_Boundaries_h
#define Vortex2d_Boundaries_h

#include <Vortex2D/Renderer/Buffer.h>
#include <Vortex2D/Renderer/Transformable.h>
#include <Vortex2D/Renderer/Work.h>
#include <Vortex2D/Renderer/RenderTexture.h>
#include <Vortex2D/Renderer/CommandBuffer.h>
#include <Vortex2D/Renderer/Drawable.h>
#include <Vortex2D/Renderer/Sprite.h>

#include <Vortex2D/Engine/Particles.h>

namespace Vortex2D { namespace Fluid {

class LevelSet;

class ObjectDrawable : public Renderer::Transformable, public Renderer::Drawable
{
};

class Polygon : public ObjectDrawable
{
public:
    // TODO number should be calculated
    Polygon(const Renderer::Device& device, std::vector<glm::vec2> points, bool inverse = false, float extent = 10.0f);

    void Initialize(const Renderer::RenderState& renderState) override;
    void Update(const glm::mat4& projection, const glm::mat4& view) override;
    void Draw(vk::CommandBuffer commandBuffer, const Renderer::RenderState& renderState) override;

private:
    const Renderer::Device& mDevice;
    uint32_t mSize;
    uint32_t mInv;
    Renderer::UniformBuffer<glm::mat4> mMVPBuffer;
    Renderer::UniformBuffer<glm::mat4> mMVBuffer;
    Renderer::VertexBuffer<glm::vec2> mVertexBuffer;
    Renderer::DescriptorSet mDescriptorSet;
    Renderer::GraphicsPipeline mPipeline;
    Renderer::Buffer<glm::vec2> mPolygonVertexBuffer;
};

class Rectangle : public Polygon
{
public:
    Rectangle(const Renderer::Device& device, const glm::vec2& size, bool inverse = false);
};

class Circle : public ObjectDrawable
{
public:
    // TODO number should be calculated
    Circle(const Renderer::Device& device, float radius, float extent = 10.0f);

    void Initialize(const Renderer::RenderState& renderState) override;
    void Update(const glm::mat4& projection, const glm::mat4& view) override;
    void Draw(vk::CommandBuffer commandBuffer, const Renderer::RenderState& renderState) override;

private:
    const Renderer::Device& mDevice;
    float mSize;
    Renderer::UniformBuffer<glm::mat4> mMVPBuffer;
    Renderer::UniformBuffer<glm::mat4> mMVBuffer;
    Renderer::VertexBuffer<glm::vec2> mVertexBuffer;
    Renderer::DescriptorSet mDescriptorSet;
    Renderer::GraphicsPipeline mPipeline;
};

extern vk::PipelineColorBlendAttachmentState IntersectionBlend;
extern vk::PipelineColorBlendAttachmentState UnionBlend;

class DistanceField : public Renderer::AbstractSprite
{
public:
    DistanceField(const Renderer::Device& device,
                  Renderer::RenderTexture& levelSet,
                  float scale = 1.0f);

    void Draw(vk::CommandBuffer commandBuffer, const Renderer::RenderState& renderState) override;

private:
    float mScale;
};

}}

#endif
