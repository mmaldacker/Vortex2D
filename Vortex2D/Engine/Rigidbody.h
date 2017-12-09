//
//  Rigidbody.h
//  Vortex2D
//

#ifndef Vortex2d_Rigidbody_h
#define Vortex2d_Rigidbody_h

#include <Vortex2D/Renderer/Buffer.h>
#include <Vortex2D/Renderer/Drawable.h>
#include <Vortex2D/Renderer/Transformable.h>
#include <Vortex2D/Renderer/Pipeline.h>

namespace Vortex2D { namespace Fluid {

class PolygonVelocity : public Renderer::Drawable, public Renderer::Transformable
{
public:
    PolygonVelocity(const Renderer::Device& device,
                    const glm::ivec2& size,
                    Renderer::GenericBuffer& valid,
                    const std::vector<glm::vec2>& points,
                    const glm::vec2& centre);

    void SetCentre(const glm::vec2& centre);
    void UpdateVelocities(const glm::vec2& velocity, float angularVelocity);

    void Initialize(const Renderer::RenderState& renderState) override;
    void Update(const glm::mat4& projection, const glm::mat4& view) override;
    void Draw(vk::CommandBuffer commandBuffer, const Renderer::RenderState& renderState) override;

private:
    struct Velocity
    {
        alignas(8) glm::vec2 velocity;
        alignas(4) float angular_velocity;
    };

    const Renderer::Device& mDevice;
    glm::ivec2 mSize;
    glm::vec2 mCentre;
    Renderer::UpdateUniformBuffer<glm::mat4> mMVPBuffer;
    Renderer::UpdateUniformBuffer<glm::mat4> mMVBuffer;
    Renderer::UpdateUniformBuffer<Velocity> mVelocity;
    Renderer::VertexBuffer<glm::vec2> mVertexBuffer;
    Renderer::DescriptorSet mDescriptorSet;
    Renderer::GraphicsPipeline mPipeline;
    uint32_t mNumVertices;
};

}}

#endif
