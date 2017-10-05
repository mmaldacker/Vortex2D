//
//  Boundaries.cpp
//  Vortex2D
//

#include "Boundaries.h"

#include <Vortex2D/Engine/LevelSet.h>
#include <Vortex2D/Engine/Particles.h>

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
    , mLocalMVPBuffer(device, vk::BufferUsageFlagBits::eStorageBuffer, true, sizeof(glm::mat4))
    , mMVPBuffer(device, vk::BufferUsageFlagBits::eStorageBuffer, false, sizeof(glm::mat4))
    , mVertexBuffer(device, vk::BufferUsageFlagBits::eStorageBuffer, false, sizeof(glm::vec2) * points.size())
    , mTransformedVertices(device, vk::BufferUsageFlagBits::eStorageBuffer, false, sizeof(glm::vec2) * points.size())
    , mUpdateCmd(device, false)
    , mRender(device, Renderer::ComputeSize::Default2D(), "../Vortex2D/PolygonDist.comp.spv")
    , mUpdate(device, {(int)points.size()}, "../Vortex2D/UpdateVertices.comp.spv")
    , mUpdateBound(mUpdate.Bind({mMVPBuffer, mVertexBuffer, mTransformedVertices}))
{
    Renderer::Buffer localVertexBuffer(device, vk::BufferUsageFlagBits::eStorageBuffer, true, sizeof(glm::vec2) * points.size());

    assert(!IsClockwise(points));

    if (inverse)
    {
        std::reverse(points.begin(), points.end());
    }

    localVertexBuffer.CopyFrom(points);

    Renderer::ExecuteCommand(device, [&](vk::CommandBuffer commandBuffer)
    {
        mVertexBuffer.CopyFrom(commandBuffer, localVertexBuffer);
    });

    mUpdateCmd.Record([&](vk::CommandBuffer commandBuffer)
    {
       mMVPBuffer.CopyFrom(commandBuffer, mLocalMVPBuffer);
       mUpdateBound.PushConstant(commandBuffer, 8, mSize);
       mUpdateBound.Record(commandBuffer);
       mTransformedVertices.Barrier(commandBuffer, vk::AccessFlagBits::eShaderWrite, vk::AccessFlagBits::eShaderRead);
    });
}

void Polygon::Initialize(LevelSet& levelSet)
{
    glm::ivec2 size(levelSet.Width, levelSet.Height);
    mRenderBounds.emplace_back(levelSet, mRender.Bind(size, {levelSet, mTransformedVertices}));
}

void Polygon::Update(const glm::mat4& view)
{
    mLocalMVPBuffer.CopyFrom(view * GetTransform());
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
        renderBoundIt->second.Record(commandBuffer);
    }
}

Rectangle::Rectangle(const Renderer::Device& device, const glm::vec2& size, bool inverse)
    : Polygon(device, {{0.0f, 0.0f}, {size.x, 0.0f}, {size.x, size.y}, {0.0f, size.y}}, inverse)
{
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

ParticleCloud::ParticleCloud(const Renderer::Device& device, Renderer::Buffer& particles, int numParticles, const glm::vec4& colour)
    : mDevice(device.Handle())
    , mMVPBuffer(device, vk::BufferUsageFlagBits::eUniformBuffer, true, sizeof(glm::mat4))
    , mColourBuffer(device, vk::BufferUsageFlagBits::eUniformBuffer, true, sizeof(glm::vec4))
    , mVertexBuffer(particles)
    , mNumVertices(numParticles)
{
    mColourBuffer.CopyFrom(colour);

    static vk::DescriptorSetLayout descriptorLayout = Renderer::DescriptorSetLayoutBuilder()
            .Binding(0, vk::DescriptorType::eUniformBuffer, vk::ShaderStageFlagBits::eVertex, 1)
            .Binding(1, vk::DescriptorType::eUniformBuffer, vk::ShaderStageFlagBits::eFragment, 1)
            .Create(device);

    mDescriptorSet = MakeDescriptorSet(device, descriptorLayout);

    Renderer::DescriptorSetUpdater(*mDescriptorSet)
            .WriteBuffers(0, 0, vk::DescriptorType::eUniformBuffer).Buffer(mMVPBuffer)
            .WriteBuffers(1, 0, vk::DescriptorType::eUniformBuffer).Buffer(mColourBuffer)
            .Update(device.Handle());

    mPipelineLayout = Renderer::PipelineLayoutBuilder()
            .DescriptorSetLayout(descriptorLayout)
            .Create(device.Handle());

    vk::ShaderModule vertexShader = device.GetShaderModule("../Vortex2D/ParticleCloud.vert.spv");
    vk::ShaderModule fragShader = device.GetShaderModule("../Vortex2D/ParticleCloud.frag.spv");

    mPipeline = Renderer::GraphicsPipeline::Builder()
            .Topology(vk::PrimitiveTopology::ePointList)
            .Shader(vertexShader, vk::ShaderStageFlagBits::eVertex)
            .Shader(fragShader, vk::ShaderStageFlagBits::eFragment)
            .VertexAttribute(0, 0, vk::Format::eR32G32Sfloat, offsetof(Particle, Position))
            .VertexBinding(0, sizeof(Particle))
            .Layout(*mPipelineLayout);
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
    mMVPBuffer.CopyFrom(projection * view * GetTransform());
}

void ParticleCloud::Draw(vk::CommandBuffer commandBuffer, const Renderer::RenderState& renderState)
{
    mPipeline.Bind(commandBuffer, renderState);
    commandBuffer.bindVertexBuffers(0, {mVertexBuffer}, {0ul});
    commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, *mPipelineLayout, 0, {*mDescriptorSet}, {});
    commandBuffer.draw(mNumVertices, 1, 0, 0);
}

}}
