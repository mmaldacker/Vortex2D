//
//  Rigidbody.cpp
//  Vortex2D
//

#include "Rigidbody.h"
#include <Vortex2D/SPIRV/Reflection.h>
#include <Vortex2D/Renderer/CommandBuffer.h>

#include "vortex2d_generated_spirv.h"

namespace Vortex2D { namespace Fluid {

PolygonVelocity::PolygonVelocity(const Renderer::Device& device,
                                 const glm::ivec2& size,
                                 Renderer::GenericBuffer& valid,
                                 const std::vector<glm::vec2>& points,
                                 const glm::vec2& centre)
    : mDevice(device)
    , mSize(size)
    , mCentre(centre)
    , mMVPBuffer(device, VMA_MEMORY_USAGE_CPU_TO_GPU)
    , mMVBuffer(device, VMA_MEMORY_USAGE_CPU_TO_GPU)
    , mVelocity(device, VMA_MEMORY_USAGE_CPU_TO_GPU)
    , mVertexBuffer(device, points.size())
    , mNumVertices(points.size())
{
    Renderer::VertexBuffer<glm::vec2> localVertices(device, points.size(), VMA_MEMORY_USAGE_CPU_ONLY);
    Renderer::CopyFrom(localVertices, points);
    Renderer::ExecuteCommand(device, [&](vk::CommandBuffer commandBuffer)
    {
        mVertexBuffer.CopyFrom(commandBuffer, localVertices);
    });

    SPIRV::Reflection reflectionVert(PolygonVelocity_vert);
    SPIRV::Reflection reflectionFrag(PolygonVelocity_frag);

    Renderer::PipelineLayout layout = {{reflectionVert, reflectionFrag}};
    mDescriptorSet = device.GetLayoutManager().MakeDescriptorSet(layout);
    Bind(device, *mDescriptorSet.descriptorSet, layout, {{mMVPBuffer, 0}, {mMVBuffer, 1}, {mVelocity, 2}, {valid, 3}});

    mPipeline = Renderer::GraphicsPipeline::Builder()
            .Topology(vk::PrimitiveTopology::eTriangleFan)
            .Shader(device.GetShaderModule(PolygonVelocity_vert), vk::ShaderStageFlagBits::eVertex)
            .Shader(device.GetShaderModule(PolygonVelocity_frag), vk::ShaderStageFlagBits::eFragment)
            .VertexAttribute(0, 0, vk::Format::eR32G32Sfloat, 0)
            .VertexBinding(0, sizeof(glm::vec2))
            .Layout(mDescriptorSet.pipelineLayout);
}

void PolygonVelocity::SetCentre(const glm::vec2& centre)
{
    mCentre = centre;
}

void PolygonVelocity::UpdateVelocities(const glm::vec2& velocity, float angularVelocity)
{
    Velocity v = {velocity, angularVelocity};
    Renderer::CopyFrom(mVelocity, v);
}

void PolygonVelocity::Initialize(const Renderer::RenderState& renderState)
{
    mPipeline.Create(mDevice.Handle(), renderState);
}

void PolygonVelocity::Update(const glm::mat4& projection, const glm::mat4& view)
{
    Renderer::CopyFrom(mMVPBuffer, projection * view * GetTransform());
    Renderer::CopyFrom(mMVBuffer, view * GetTransform());
}

void PolygonVelocity::Draw(vk::CommandBuffer commandBuffer, const Renderer::RenderState& renderState)
{
    mPipeline.Bind(commandBuffer, renderState);
    commandBuffer.pushConstants(mDescriptorSet.pipelineLayout, vk::ShaderStageFlagBits::eVertex, 0, sizeof(mCentre), &mCentre);
    commandBuffer.pushConstants(mDescriptorSet.pipelineLayout, vk::ShaderStageFlagBits::eFragment, 8, sizeof(mSize), &mSize);
    commandBuffer.bindVertexBuffers(0, {mVertexBuffer.Handle()}, {0ul});
    commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics,
                                     mDescriptorSet.pipelineLayout, 0, {*mDescriptorSet.descriptorSet}, {});
    commandBuffer.draw(mNumVertices, 1, 0, 0);
}

}}
