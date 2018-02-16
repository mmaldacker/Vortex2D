//
//  Velocity.h
//  Vortex2D
//

#ifndef Vortex_Velocity_h
#define Vortex_Velocity_h

#include <Vortex2D/Renderer/Device.h>
#include <Vortex2D/Renderer/Texture.h>
#include <Vortex2D/Renderer/RenderTexture.h>

namespace Vortex2D { namespace Fluid {

class Velocity
{
public:
    Velocity(const Renderer::Device& device, const glm::ivec2& size);

    Renderer::RenderTexture& Input();
    Renderer::Texture& Output();

    void CopyBack(vk::CommandBuffer commandBuffer);
    void Clear(vk::CommandBuffer commandBuffer);

private:
    Renderer::RenderTexture mInputVelocity;
    Renderer::Texture mOutputVelocity;
};

}}

#endif
