//
//  Boundaries.cpp
//  Vortex2D
//

#include "Boundaries.h"

#include <Vortex2D/Engine/LevelSet.h>
#include <Vortex2D/Engine/Particles.h>
#include <Vortex2D/SPIRV/Reflection.h>

#include <glm/gtx/transform.hpp>

namespace Vortex2D { namespace Fluid {

namespace
{
const int extent = 3;

bool IsClockwise(const std::vector<glm::vec2>& points)
{
    float total = 0.0f;
    for (int i = points.size() - 1, j = 0; j < points.size(); i = j++)
    {
        total += (points[j].x - points[i].x) * (points[i].y + points[j].y);
    }

    return total > 0.0;
}

std::vector<glm::vec2> GetBoundingBox(const std::vector<glm::vec2>& points, int extent)
{
    glm::vec2 topLeft(std::numeric_limits<float>::max());
    glm::vec2 bottomRight(std::numeric_limits<float>::min());

    for (auto& point: points)
    {
        topLeft.x = glm::min(topLeft.x, point.x);
        topLeft.y = glm::min(topLeft.y, point.y);

        bottomRight.x = glm::max(bottomRight.x, point.x);
        bottomRight.y = glm::max(bottomRight.y, point.y);
    }

    topLeft -= glm::vec2(extent);
    bottomRight += glm::vec2(extent);

    return {{topLeft.x, topLeft.y},
            {bottomRight.x, topLeft.y},
            {topLeft.x, bottomRight.y},
            {bottomRight.x, topLeft.y,},
            {bottomRight.x, bottomRight.y},
            {topLeft.x, bottomRight.y}};
}

}

Polygon::Polygon(const Renderer::Device& device, std::vector<glm::vec2> points, bool inverse, int extent)
    : mDevice(device)
    , mSize(points.size())
    , mInv(inverse)
    , mMVPBuffer(device, VMA_MEMORY_USAGE_CPU_TO_GPU)
    , mMVBuffer(device, VMA_MEMORY_USAGE_CPU_TO_GPU)
    , mVertexBuffer(device, 6)
    , mPolygonVertexBuffer(device, points.size())
{
    assert(!IsClockwise(points));
    if (inverse)
    {
        std::reverse(points.begin(), points.end());
    }

    Renderer::Buffer<glm::vec2> localPolygonVertexBuffer(device, points.size(), VMA_MEMORY_USAGE_CPU_ONLY);
    Renderer::CopyFrom(localPolygonVertexBuffer, points);

    auto boundingBox = GetBoundingBox(points, extent);
    Renderer::Buffer<glm::vec2> localVertexBuffer(device, boundingBox.size(), VMA_MEMORY_USAGE_CPU_ONLY);
    Renderer::CopyFrom(localVertexBuffer, boundingBox);

    Renderer::ExecuteCommand(device, [&](vk::CommandBuffer commandBuffer)
    {
        mPolygonVertexBuffer.CopyFrom(commandBuffer, localPolygonVertexBuffer);
        mVertexBuffer.CopyFrom(commandBuffer, localVertexBuffer);
    });

    SPIRV::Reflection reflectionVert(device.GetShaderSPIRV("../Vortex2D/Position.vert.spv"));
    SPIRV::Reflection reflectionFrag(device.GetShaderSPIRV("../Vortex2D/PolygonDist.frag.spv"));

    Renderer::PipelineLayout layout = {{reflectionVert, reflectionFrag}};
    mDescriptorSet = device.GetLayoutManager().MakeDescriptorSet(layout);
    Bind(device, *mDescriptorSet.descriptorSet, layout, {{mMVPBuffer}, {mMVBuffer}, {mPolygonVertexBuffer}});

    mPipeline = Renderer::GraphicsPipeline::Builder()
            .Topology(vk::PrimitiveTopology::eTriangleList)
            .Shader(device.GetShaderModule("../Vortex2D/Position.vert.spv"), vk::ShaderStageFlagBits::eVertex)
            .Shader(device.GetShaderModule("../Vortex2D/PolygonDist.frag.spv"), vk::ShaderStageFlagBits::eFragment)
            .VertexAttribute(0, 0, vk::Format::eR32G32Sfloat, 0)
            .VertexBinding(0, sizeof(glm::vec2))
            .Layout(mDescriptorSet.pipelineLayout);
}

void Polygon::Initialize(const Renderer::RenderState& renderState)
{
    mPipeline.Create(mDevice.Handle(), renderState);
}

void Polygon::Update(const glm::mat4& projection, const glm::mat4& view)
{
    Renderer::CopyFrom(mMVPBuffer, projection * view * GetTransform());
    Renderer::CopyFrom(mMVBuffer, view * GetTransform());
}

void Polygon::Draw(vk::CommandBuffer commandBuffer, const Renderer::RenderState& renderState)
{
    mPipeline.Bind(commandBuffer, renderState);
    commandBuffer.pushConstants(mDescriptorSet.pipelineLayout, vk::ShaderStageFlagBits::eFragment, 0, 4, &mSize);
    commandBuffer.pushConstants(mDescriptorSet.pipelineLayout, vk::ShaderStageFlagBits::eFragment, 4, 4, &mInv);
    commandBuffer.bindVertexBuffers(0, {mVertexBuffer.Handle()}, {0ul});
    commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics,
                                     mDescriptorSet.pipelineLayout, 0, {*mDescriptorSet.descriptorSet}, {});
    commandBuffer.draw(6, 1, 0, 0);
}

Rectangle::Rectangle(const Renderer::Device& device, const glm::vec2& size, bool inverse)
    : Polygon(device, {{0.0f, 0.0f}, {size.x, 0.0f}, {size.x, size.y}, {0.0f, size.y}}, inverse)
{
}

Circle::Circle(const Renderer::Device& device, float radius, int extent)
    : mDevice(device)
    , mSize(radius)
    , mMVPBuffer(device, VMA_MEMORY_USAGE_CPU_TO_GPU)
    , mMVBuffer(device, VMA_MEMORY_USAGE_CPU_TO_GPU)
    , mVertexBuffer(device, 6)
{
    std::vector<glm::vec2> points = {{-radius, -radius}, {radius, -radius}, {radius, radius}, {-radius, radius}};

    auto boundingBox = GetBoundingBox(points, extent);
    Renderer::Buffer<glm::vec2> localVertexBuffer(device, boundingBox.size(), VMA_MEMORY_USAGE_CPU_ONLY);
    Renderer::CopyFrom(localVertexBuffer, boundingBox);

    Renderer::ExecuteCommand(device, [&](vk::CommandBuffer commandBuffer)
    {
        mVertexBuffer.CopyFrom(commandBuffer, localVertexBuffer);
    });

    SPIRV::Reflection reflectionVert(device.GetShaderSPIRV("../Vortex2D/Position.vert.spv"));
    SPIRV::Reflection reflectionFrag(device.GetShaderSPIRV("../Vortex2D/CircleDist.frag.spv"));

    Renderer::PipelineLayout layout = {{reflectionVert, reflectionFrag}};
    mDescriptorSet = device.GetLayoutManager().MakeDescriptorSet(layout);
    Bind(device, *mDescriptorSet.descriptorSet, layout, {{mMVPBuffer}, {mMVBuffer}});

    mPipeline = Renderer::GraphicsPipeline::Builder()
            .Topology(vk::PrimitiveTopology::eTriangleList)
            .Shader(device.GetShaderModule("../Vortex2D/Position.vert.spv"), vk::ShaderStageFlagBits::eVertex)
            .Shader(device.GetShaderModule("../Vortex2D/CircleDist.frag.spv"), vk::ShaderStageFlagBits::eFragment)
            .VertexAttribute(0, 0, vk::Format::eR32G32Sfloat, 0)
            .VertexBinding(0, sizeof(glm::vec2))
            .Layout(mDescriptorSet.pipelineLayout);
}

void Circle::Initialize(const Renderer::RenderState& renderState)
{
    mPipeline.Create(mDevice.Handle(), renderState);
}

void Circle::Update(const glm::mat4& projection, const glm::mat4& view)
{
    Renderer::CopyFrom(mMVPBuffer, projection * view * GetTransform());
    Renderer::CopyFrom(mMVBuffer, view * GetTransform());
}

void Circle::Draw(vk::CommandBuffer commandBuffer, const Renderer::RenderState& renderState)
{
    mPipeline.Bind(commandBuffer, renderState);
    commandBuffer.pushConstants(mDescriptorSet.pipelineLayout, vk::ShaderStageFlagBits::eFragment, 0, 4, &mSize);
    commandBuffer.bindVertexBuffers(0, {mVertexBuffer.Handle()}, {0ul});
    commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics,
                                     mDescriptorSet.pipelineLayout, 0, {*mDescriptorSet.descriptorSet}, {});
    commandBuffer.draw(6, 1, 0, 0);
}

vk::PipelineColorBlendAttachmentState IntersectionBlend = vk::PipelineColorBlendAttachmentState()
        .setBlendEnable(true)
        .setColorBlendOp(vk::BlendOp::eMax)
        .setSrcColorBlendFactor(vk::BlendFactor::eOne)
        .setDstColorBlendFactor(vk::BlendFactor::eOne)
        .setColorWriteMask(vk::ColorComponentFlagBits::eR);

vk::PipelineColorBlendAttachmentState UnionBlend = vk::PipelineColorBlendAttachmentState()
        .setBlendEnable(true)
        .setColorBlendOp(vk::BlendOp::eMin)
        .setSrcColorBlendFactor(vk::BlendFactor::eOne)
        .setDstColorBlendFactor(vk::BlendFactor::eOne)
        .setColorWriteMask(vk::ColorComponentFlagBits::eR);

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
    , mMVPBuffer(device, VMA_MEMORY_USAGE_CPU_TO_GPU)
    , mColourBuffer(device, VMA_MEMORY_USAGE_CPU_TO_GPU)
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
