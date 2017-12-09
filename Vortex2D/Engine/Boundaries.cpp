//
//  Boundaries.cpp
//  Vortex2D
//

#include "Boundaries.h"

#include <Vortex2D/Engine/LevelSet.h>
#include <Vortex2D/Engine/Particles.h>
#include <Vortex2D/SPIRV/Reflection.h>

namespace Vortex2D { namespace Fluid {

namespace
{
bool IsClockwise(const std::vector<glm::vec2>& points)
{
    float total = 0.0f;
    for (int i = points.size() - 1, j = 0; j < points.size(); i = j++)
    {
        total += (points[j].x - points[i].x) * (points[j].y + points[j].y);
    }

    return total > 0.0;
}
}

Polygon::Polygon(const Renderer::Device& device, std::vector<glm::vec2> points, bool inverse)
    : mSize(points.size())
    , mInverse(inverse)
    , mMVPBuffer(device)
    , mVertexBuffer(device, points.size())
    , mTransformedVertices(device, points.size())
    , mUpdateCmd(device, false)
    , mRender(device, Renderer::ComputeSize::Default2D(), "../Vortex2D/PolygonDist.comp.spv")
    , mUpdate(device, {(int)points.size()}, "../Vortex2D/UpdateVertices.comp.spv")
    , mUpdateBound(mUpdate.Bind({mMVPBuffer, mVertexBuffer, mTransformedVertices}))
{
    assert(!IsClockwise(points));
    if (inverse)
    {
        std::reverse(points.begin(), points.end());
    }

    Renderer::Buffer<glm::vec2> localVertexBuffer(device, points.size(), true);
    Renderer::CopyFrom(localVertexBuffer, points);
    Renderer::ExecuteCommand(device, [&](vk::CommandBuffer commandBuffer)
    {
        mVertexBuffer.CopyFrom(commandBuffer, localVertexBuffer);
    });

    mUpdateCmd.Record([&](vk::CommandBuffer commandBuffer)
    {
        mMVPBuffer.Upload(commandBuffer);
        mUpdateBound.PushConstant(commandBuffer, 8, mSize);
        mUpdateBound.Record(commandBuffer);
        mTransformedVertices.Barrier(commandBuffer, vk::AccessFlagBits::eShaderWrite, vk::AccessFlagBits::eShaderRead);
    });
}

void Polygon::Initialize(LevelSet& levelSet)
{
    glm::ivec2 size(levelSet.Width, levelSet.Height);

    auto renderBoundIt = std::find_if(mRenderBounds.begin(), mRenderBounds.end(), [&](const auto& other)
    {
        return other.first == levelSet;
    });

    if (renderBoundIt == mRenderBounds.end())
    {
        mRenderBounds.emplace_back(levelSet, mRender.Bind(size, {levelSet, mTransformedVertices}));
    }
}

void Polygon::Update(const glm::mat4& view)
{
    Renderer::CopyFrom(mMVPBuffer, view * GetTransform());
    mUpdateCmd.Submit();
}

void Polygon::Draw(vk::CommandBuffer commandBuffer, LevelSet& levelSet)
{
    auto renderBoundIt = std::find_if(mRenderBounds.begin(), mRenderBounds.end(), [&](const auto& other)
    {
        return other.first == levelSet;
    });

    if (renderBoundIt != mRenderBounds.end())
    {
        renderBoundIt->second.PushConstant(commandBuffer, 8, mSize);
        renderBoundIt->second.PushConstant(commandBuffer, 12, mInverse ? 1 : 0);
        renderBoundIt->second.Record(commandBuffer);
    }
}

Rectangle::Rectangle(const Renderer::Device& device, const glm::vec2& size, bool inverse)
    : Polygon(device, {{0.0f, 0.0f}, {size.x, 0.0f}, {size.x, size.y}, {0.0f, size.y}}, inverse)
{
}

Circle::Circle(const Renderer::Device& device, float radius)
    : mSize(radius)
    , mMVPBuffer(device)
    , mVertexBuffer(device)
    , mTransformedVertices(device)
    , mUpdateCmd(device, false)
    , mRender(device, Renderer::ComputeSize::Default2D(), "../Vortex2D/CircleDist.comp.spv")
    , mUpdate(device, {1}, "../Vortex2D/UpdateVertices.comp.spv")
    , mUpdateBound(mUpdate.Bind({mMVPBuffer, mVertexBuffer, mTransformedVertices}))
{
    mUpdateCmd.Record([&](vk::CommandBuffer commandBuffer)
    {
        mMVPBuffer.Upload(commandBuffer);
        mUpdateBound.PushConstant(commandBuffer, 8, 1);
        mUpdateBound.Record(commandBuffer);
        mTransformedVertices.Barrier(commandBuffer, vk::AccessFlagBits::eShaderWrite, vk::AccessFlagBits::eShaderRead);
    });
}

void Circle::Initialize(LevelSet& levelSet)
{
    glm::ivec2 size(levelSet.Width, levelSet.Height);

    auto renderBoundIt = std::find_if(mRenderBounds.begin(), mRenderBounds.end(), [&](const auto& other)
    {
        return other.first == levelSet;
    });

    if (renderBoundIt == mRenderBounds.end())
    {
        mRenderBounds.emplace_back(levelSet, mRender.Bind(size, {mMVPBuffer, levelSet, mTransformedVertices}));
    }
}

void Circle::Update(const glm::mat4& view)
{
    Renderer::CopyFrom(mMVPBuffer, view * GetTransform());
    mUpdateCmd.Submit();
}

void Circle::Draw(vk::CommandBuffer commandBuffer, LevelSet& levelSet)
{
    auto renderBoundIt = std::find_if(mRenderBounds.begin(), mRenderBounds.end(), [&](const auto& other)
    {
        return other.first == levelSet;
    });

    if (renderBoundIt != mRenderBounds.end())
    {
        renderBoundIt->second.PushConstant(commandBuffer, 8, mSize);
        renderBoundIt->second.Record(commandBuffer);
    }
}


DistanceField::DistanceField(const Renderer::Device& device,
                             LevelSet& levelSet,
                             const glm::vec4& colour,
                             float scale)
    : Renderer::AbstractSprite(device,
                               "../Vortex2D/DistanceField.frag.spv",
                               levelSet)
    , mColour(colour)
    , mScale(scale)
{
}

void DistanceField::Draw(vk::CommandBuffer commandBuffer, const Renderer::RenderState& renderState)
{
    PushConstant(commandBuffer, 0, mColour);
    PushConstant(commandBuffer, 16, mScale);
    AbstractSprite::Draw(commandBuffer, renderState);
}

ParticleCloud::ParticleCloud(const Renderer::Device& device, Renderer::GenericBuffer& particles, int numParticles, const glm::vec4& colour)
    : mDevice(device.Handle())
    , mMVPBuffer(device)
    , mColourBuffer(device)
    , mVertexBuffer(particles)
    , mNumVertices(numParticles)
{
    Renderer::CopyFrom(mColourBuffer, colour);

    SPIRV::Reflection reflectionVert(device.GetShaderSPIRV("../Vortex2D/ParticleCloud.vert.spv"));
    SPIRV::Reflection reflectionFrag(device.GetShaderSPIRV("../Vortex2D/ParticleCloud.frag.spv"));

    Renderer::PipelineLayout layout = {{reflectionVert, reflectionFrag}};
    mDescriptorSet = device.GetLayoutManager().MakeDescriptorSet(layout);
    Bind(device, *mDescriptorSet.descriptorSet, layout, {{mMVPBuffer}, {mColourBuffer}});

    mPipeline = Renderer::GraphicsPipeline::Builder()
            .Topology(vk::PrimitiveTopology::ePointList)
            .Shader(device.GetShaderModule("../Vortex2D/ParticleCloud.vert.spv"), vk::ShaderStageFlagBits::eVertex)
            .Shader(device.GetShaderModule("../Vortex2D/ParticleCloud.frag.spv"), vk::ShaderStageFlagBits::eFragment)
            .VertexAttribute(0, 0, vk::Format::eR32G32Sfloat, offsetof(Particle, Position))
            .VertexBinding(0, sizeof(Particle))
            .Layout(mDescriptorSet.pipelineLayout);
}

void ParticleCloud::SetNumParticles(int numParticles)
{
    mNumVertices = numParticles;
}

void ParticleCloud::Initialize(const Renderer::RenderState& renderState)
{
    mPipeline.Create(mDevice, renderState);
}

void ParticleCloud::Update(const glm::mat4& projection, const glm::mat4& view)
{
    Renderer::CopyFrom(mMVPBuffer, projection * view * GetTransform());
}

void ParticleCloud::Draw(vk::CommandBuffer commandBuffer, const Renderer::RenderState& renderState)
{
    mPipeline.Bind(commandBuffer, renderState);
    commandBuffer.bindVertexBuffers(0, {mVertexBuffer.Handle()}, {0ul});
    commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics,
                                     mDescriptorSet.pipelineLayout, 0, {*mDescriptorSet.descriptorSet}, {});
    commandBuffer.draw(mNumVertices, 1, 0, 0);
}

}}
