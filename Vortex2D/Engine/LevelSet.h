//
//  LevelSet.h
//  Vortex2D
//

#ifndef LevelSet_h
#define LevelSet_h

#include <Vortex2D/Renderer/RenderTexture.h>
#include <Vortex2D/Renderer/Pipeline.h>
#include <Vortex2D/Renderer/DescriptorSet.h>

namespace Vortex2D { namespace Fluid {

/**
 * @brief A LevelSet class, which extends the Buffer class, with one dimension
 * The additional functionality ensures that the buffer represents a distance set.
 * This set can be reinitialised (or improved).
 */
class LevelSet : public Renderer::RenderTexture
{
public:
    LevelSet(const Renderer::Device& device, const glm::vec2& size);

    /**
     * @brief Iterative improvements to transform buffer into distance function
     * @param iterations number of iterations (1-3 after advection, 100 for reinitialisation)
     */
    void Redistance(int iterations);

    void Extrapolate(Renderer::Buffer& solidPhi);

private:
    Renderer::Texture mLevelSet0;
    Renderer::Texture mLevelSetBack;

    vk::UniqueDescriptorSet mExtrapolateDescriptorSet;
    Renderer::PipelineLayout mExtrapolateLayout;
    vk::UniquePipeline mExtrapolatePipeline;

    vk::UniqueDescriptorSet mRedistanceFrontDescriptorSet;
    vk::UniqueDescriptorSet mRedistanceBackDescriptorSet;
    Renderer::PipelineLayout mRedistanceLayout;
    vk::UniquePipeline mRedistancePipeline;
};

}}

#endif
