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

namespace Vortex2D { namespace Renderer {

class RenderTarget;

class Sprite : public Drawable
{
public:
    Sprite(const Device& device, const Texture& texture);

    void Create(RenderTarget& renderTarget);

    void Update(const glm::mat4& mvp);

    void Draw(vk::CommandBuffer commandBuffer, vk::RenderPass renderPass) override;
private:
    struct Vertex
    {
        glm::vec2 uv;
        glm::vec2 pos;
    };

    Buffer mMVPBuffer;
    Buffer mVertexBuffer;
    vk::UniqueSampler mSampler;
    DescriptorSet mDescriptorSet;
    PipelineLayout mPipelineLayout;
    GraphicsPipeline mPipeline;
};

}}

#endif
