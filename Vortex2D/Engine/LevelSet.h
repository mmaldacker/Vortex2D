//
//  LevelSet.h
//  Vortex2D
//

#ifndef LevelSet_h
#define LevelSet_h

#include <Vortex2D/Renderer/RenderTexture.h>
#include <Vortex2D/Renderer/Work.h>
#include <Vortex2D/Renderer/CommandBuffer.h>

namespace Vortex2D { namespace Fluid {

class SignedObject;

/**
 * @brief A LevelSet class, which extends the Buffer class, with one dimension
 * The additional functionality ensures that the buffer represents a distance set.
 * This set can be reinitialised (or improved).
 */
class LevelSet : public Renderer::RenderTexture
{
public:
    LevelSet(const Renderer::Device& device, const glm::ivec2& size, int reinitializeIterations = 100);

    void ExtrapolateInit(Renderer::Texture& solidPhi);

    void Reinitialise();
    void Extrapolate();
    void ExtrapolateRecord(vk::CommandBuffer commandBuffer);

    using DrawableList = std::initializer_list<std::reference_wrapper<SignedObject>>;

    void DrawSignedObject(DrawableList drawables);
    void SubmitSignedBoject();

private:
    Renderer::Texture mLevelSet0;
    Renderer::Texture mLevelSetBack;

    vk::UniqueSampler mSampler;

    Renderer::Work mExtrapolate;
    Renderer::Work::Bound mExtrapolateBound;
    Renderer::Work mRedistance;
    Renderer::Work::Bound mRedistanceFront;
    Renderer::Work::Bound mRedistanceBack;

    Renderer::CommandBuffer mExtrapolateCmd;
    Renderer::CommandBuffer mReinitialiseCmd;
    Renderer::CommandBuffer mSignedObjectCmd;
};

}}

#endif
