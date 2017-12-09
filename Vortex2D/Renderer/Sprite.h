//
//  Sprite.h
//  Vortex2D
//

#ifndef Sprite_h
#define Sprite_h

#include <Vortex2D/Renderer/Drawable.h>
#include <Vortex2D/Renderer/Texture.h>
#include <Vortex2D/Renderer/Pipeline.h>
#include <Vortex2D/Renderer/Buffer.h>
#include <Vortex2D/Renderer/DescriptorSet.h>
#include <Vortex2D/Renderer/Transformable.h>

namespace Vortex2D { namespace Renderer {

struct RenderTarget;

class AbstractSprite : public Drawable, public Transformable
{
public:
    AbstractSprite(const Device& device, const std::string& fragShaderName, Texture& texture);

    void Initialize(const RenderState& renderState) override;
    void Update(const glm::mat4& projection, const glm::mat4& view) override;
    void Draw(vk::CommandBuffer commandBuffer, const RenderState& renderState) override;

    template<typename T>
    void PushConstant(vk::CommandBuffer commandBuffer, uint32_t offset, const T& data)
    {
        commandBuffer.pushConstants(mDescriptorSet.pipelineLayout, vk::ShaderStageFlagBits::eFragment, offset, sizeof(T), &data);
    }

private:
    struct Vertex
    {
        glm::vec2 uv;
        glm::vec2 pos;
    };

    const Device& mDevice;
    UpdateBuffer<UniformBuffer, glm::mat4> mMVPBuffer;
    VertexBuffer<Vertex> mVertexBuffer;
    vk::UniqueSampler mSampler;
    DescriptorSet mDescriptorSet;
    GraphicsPipeline mPipeline;
};

class Sprite : public AbstractSprite
{
public:
    Sprite(const Device& device, Texture& texture);
};

}}

#endif
