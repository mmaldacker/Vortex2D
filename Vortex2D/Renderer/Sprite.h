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
    AbstractSprite(const Device& device, const std::string& fragShaderName, const Texture& texture);

    void Initialize(const RenderState& renderState) override;
    void Update(const glm::mat4& model, const glm::mat4& view) override;
    void Draw(vk::CommandBuffer commandBuffer, const RenderState& renderState) override;

private:
    struct Vertex
    {
        glm::vec2 uv;
        glm::vec2 pos;
    };

    vk::Device mDevice;
    Buffer mMVPBuffer;
    Buffer mVertexBuffer;
    vk::UniqueSampler mSampler;
    vk::UniqueDescriptorSet mDescriptorSet;
    vk::UniquePipelineLayout mPipelineLayout;
    GraphicsPipeline mPipeline;
};

class Sprite : public AbstractSprite
{
public:
    Sprite(const Device& device, const Texture& texture);
};

}}

#endif
