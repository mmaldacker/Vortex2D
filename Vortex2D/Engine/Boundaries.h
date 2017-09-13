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
#include <Vortex2D/Renderer/Pipeline.h>
#include <Vortex2D/Renderer/Sprite.h>

namespace Vortex2D { namespace Fluid {

class LevelSet;

class SignedObject
{
public:
    virtual void Initialize(LevelSet& levelSet) = 0;
    virtual void Update(const glm::mat4& view) = 0;
    virtual void Draw(vk::CommandBuffer commandBuffer, LevelSet& levelSet) = 0;
};

class Polygon : public Renderer::Transformable, public SignedObject
{
public:
    Polygon(const Renderer::Device& device, std::vector<glm::vec2> points, bool inverse = false);

    void Initialize(LevelSet& levelSet) override;
    void Update(const glm::mat4& view) override;
    void Draw(vk::CommandBuffer commandBuffer, LevelSet& levelSet) override;

private:
    int mSize;
    Renderer::Buffer mLocalMVPBuffer;
    Renderer::Buffer mMVPBuffer;
    Renderer::Buffer mVertexBuffer;
    Renderer::Buffer mTransformedVertices;
    Renderer::CommandBuffer mUpdateCmd;

    Renderer::Work mRender;
    std::vector<std::pair<Renderer::RenderTexture&, Renderer::Work::Bound>> mRenderBounds;
    Renderer::Work mUpdate;
    Renderer::Work::Bound mUpdateBound;
};

class Rectangle : public Polygon
{
public:
    Rectangle(const Renderer::Device& device, const glm::vec2& size, bool inverse = false);
};

class DistanceField : public Renderer::AbstractSprite
{
public:
    DistanceField(const Renderer::Device& device, LevelSet& levelSet);
};

}}

#endif
