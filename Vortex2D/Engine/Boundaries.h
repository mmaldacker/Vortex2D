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

class Polygon : public Renderer::Transformable, public Renderer::Drawable
{
public:
    Polygon(const Renderer::Device& device, std::vector<glm::vec2> points, bool inverse = false, int extent = 3);

    void Initialize(const Renderer::RenderState& renderState) override;
    void Update(const glm::mat4& projection, const glm::mat4& view) override;
    void Draw(vk::CommandBuffer commandBuffer, const Renderer::RenderState& renderState) override;

private:
    const Renderer::Device& mDevice;
    uint32_t mSize;
    uint32_t mInv;
    Renderer::UpdateBuffer<Renderer::UniformBuffer, glm::mat4> mMVPBuffer;
    Renderer::UpdateUniformBuffer<glm::mat4> mMVBuffer;
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

// TODO a lot of duplication between Ellipse and Polygon (and the Shapes classes?)
class Circle : public Renderer::Transformable, public Renderer::Drawable
{
public:
    Circle(const Renderer::Device& device, float radius);

    void Initialize(const Renderer::RenderState& renderState) override;
    void Update(const glm::mat4& projection, const glm::mat4& view) override;
    void Draw(vk::CommandBuffer commandBuffer, const Renderer::RenderState& renderState) override;

private:
    const Renderer::Device& mDevice;
    float mSize;
    Renderer::UpdateBuffer<Renderer::UniformBuffer, glm::mat4> mMVPBuffer;
    Renderer::UpdateUniformBuffer<glm::mat4> mMVBuffer;
    Renderer::VertexBuffer<glm::vec2> mVertexBuffer;
    Renderer::DescriptorSet mDescriptorSet;
    Renderer::GraphicsPipeline mPipeline;
};

// TODO have colour has member variable and updated in the Update function
class DistanceField : public Renderer::AbstractSprite
{
public:
    DistanceField(const Renderer::Device& device,
                  LevelSet& levelSet,
                  const glm::vec4& colour,
                  float scale = 1.0f);

    void Draw(vk::CommandBuffer commandBuffer, const Renderer::RenderState& renderState) override;

private:
    glm::vec4 mColour;
    float mScale;
};

class ParticleCloud : public Renderer::Drawable, public Renderer::Transformable
{
public:
    ParticleCloud(const Renderer::Device& device, Renderer::GenericBuffer& particles, int numParticles, const glm::vec4& colour);

    void SetNumParticles(int numParticles);

    void Initialize(const Renderer::RenderState& renderState) override;
    void Update(const glm::mat4& projection, const glm::mat4& view) override;
    void Draw(vk::CommandBuffer commandBuffer, const Renderer::RenderState& renderState) override;

private:
    vk::Device mDevice;
    Renderer::UpdateUniformBuffer<glm::mat4> mMVPBuffer;
    Renderer::UpdateUniformBuffer<glm::vec4> mColourBuffer;
    Renderer::GenericBuffer& mVertexBuffer;
    Renderer::DescriptorSet mDescriptorSet;
    Renderer::GraphicsPipeline mPipeline;
    uint32_t mNumVertices;
};

}}

#endif
