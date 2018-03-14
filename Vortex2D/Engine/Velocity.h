//
//  Velocity.h
//  Vortex2D
//

#ifndef Vortex2d_Velocity_h
#define Vortex2d_Velocity_h

#include <Vortex2D/Renderer/Work.h>
#include <Vortex2D/Renderer/Device.h>
#include <Vortex2D/Renderer/Texture.h>
#include <Vortex2D/Renderer/RenderTexture.h>
#include <Vortex2D/Renderer/CommandBuffer.h>

namespace Vortex2D { namespace Fluid {

class Velocity
{
public:
    Velocity(const Renderer::Device& device, const glm::ivec2& size);

    Renderer::RenderTexture& Input();
    Renderer::Texture& Output();
    Renderer::Texture& D();

    void CopyBack(vk::CommandBuffer commandBuffer);
    void Clear(vk::CommandBuffer commandBuffer);

    void SaveCopy();
    void VelocityDiff();

private:
    Renderer::RenderTexture mInputVelocity;
    Renderer::Texture mOutputVelocity;
    Renderer::Texture mDVelocity;

    Renderer::Work mVelocityDiff;
    Renderer::Work::Bound mVelocityDiffBound;

    Renderer::CommandBuffer mSaveCopyCmd;
    Renderer::CommandBuffer mVelocityDiffCmd;
};

}}

#endif
