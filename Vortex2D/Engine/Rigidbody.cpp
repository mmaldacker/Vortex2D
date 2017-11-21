//
//  Rigidbody.cpp
//  Vortex2D
//

#include "Rigidbody.h"
#include <Vortex2D/SPIRV/Reflection.h>
#include <Vortex2D/Renderer/CommandBuffer.h>

namespace Vortex2D { namespace Fluid {

PolygonVelocity::PolygonVelocity(const Renderer::Device& device,
                                 const glm::ivec2& size,
                                 Renderer::GenericBuffer& valid,
                                 const std::vector<glm::vec2>& points,
                                 const glm::vec2& centre)
    : mDevice(device)
    , mSize(size)
    , mCentre(centre)
    , mMVPBuffer(device)
    , mMVBuffer(device)
    , mVelocity(device)
    , mVertexBuffer(device, points.size())
    , mNumVertices(points.size())
{
    Renderer::VertexBuffer<glm::vec2> localVertices(device, points.size(), true);
    Renderer::CopyFrom(localVertices, points);
    Renderer::ExecuteCommand(device, [&](vk::CommandBuffer commandBuffer)
    {
        mVertexBuffer.CopyFrom(commandBuffer, localVertices);
    });

    SPIRV::Reflection reflectionVert(device.GetShaderSPIRV("../Vortex2D/PolygonVelocity.vert.spv"));
    SPIRV::Reflection reflectionFrag(device.GetShaderSPIRV("../Vortex2D/PolygonVelocity.frag.spv"));

    vk::DescriptorSetLayout descriptorLayout = Renderer::DescriptorSetLayoutBuilder()
            .Binding(reflectionVert.GetDescriptorTypesMap(), reflectionVert.GetShaderStage())
            .Binding(reflectionFrag.GetDescriptorTypesMap(), reflectionFrag.GetShaderStage())
            .Create(device);

    mDescriptorSet = MakeDescriptorSet(device, descriptorLayout);

    Renderer::DescriptorSetUpdater(*mDescriptorSet)
            .Bind(reflectionVert.GetDescriptorTypesMap(), {{mMVPBuffer, 0}, {mMVBuffer, 1}})
            .Bind(reflectionFrag.GetDescriptorTypesMap(), {{mVelocity, 2}, {valid, 3}})
            .Update(device.Handle());

    mPipelineLayout = Renderer::PipelineLayoutBuilder()
            .PushConstantRange({reflectionVert.GetShaderStage(), 0, reflectionVert.GetPushConstantsSize()})
            .PushConstantRange({reflectionFrag.GetShaderStage(), 8, reflectionFrag.GetPushConstantsSize()})
            .DescriptorSetLayout(descriptorLayout)
            .Create(device.Handle());

    vk::ShaderModule vertexShader = device.GetShaderModule("../Vortex2D/PolygonVelocity.vert.spv");
    vk::ShaderModule fragShader = device.GetShaderModule("../Vortex2D/PolygonVelocity.frag.spv");

    mPipeline = Renderer::GraphicsPipeline::Builder()
            .Topology(vk::PrimitiveTopology::eTriangleFan)
            .Shader(vertexShader, vk::ShaderStageFlagBits::eVertex)
            .Shader(fragShader, vk::ShaderStageFlagBits::eFragment)
            .VertexAttribute(0, 0, vk::Format::eR32G32Sfloat, 0)
            .VertexBinding(0, sizeof(glm::vec2))
            .Layout(*mPipelineLayout);
}

void PolygonVelocity::SetCentre(const glm::vec2& centre)
{
    mCentre = centre;
}

void PolygonVelocity::UpdateVelocities(const glm::vec2& velocity, float angularVelocity)
{
    Velocity v = {velocity, angularVelocity};
    Renderer::CopyFrom(mVelocity, v);
    ExecuteCommand(mDevice, [&](vk::CommandBuffer commandBuffer)
    {
        mVelocity.Upload(commandBuffer);
    });
}

void PolygonVelocity::Initialize(const Renderer::RenderState& renderState)
{
    mPipeline.Create(mDevice.Handle(), renderState);
}

void PolygonVelocity::Update(const glm::mat4& projection, const glm::mat4& view)
{
    Renderer::CopyFrom(mMVPBuffer, projection * view * GetTransform());
    Renderer::CopyFrom(mMVBuffer, view * GetTransform());
    ExecuteCommand(mDevice, [&](vk::CommandBuffer commandBuffer)
    {
       mMVPBuffer.Upload(commandBuffer);
       mMVBuffer.Upload(commandBuffer);
    });
}

void PolygonVelocity::Draw(vk::CommandBuffer commandBuffer, const Renderer::RenderState& renderState)
{
    mPipeline.Bind(commandBuffer, renderState);
    commandBuffer.pushConstants(*mPipelineLayout, vk::ShaderStageFlagBits::eVertex, 0, sizeof(mCentre), &mCentre);
    commandBuffer.pushConstants(*mPipelineLayout, vk::ShaderStageFlagBits::eFragment, 8, sizeof(mSize), &mSize);
    commandBuffer.bindVertexBuffers(0, {mVertexBuffer.Handle()}, {0ul});
    commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, *mPipelineLayout, 0, {*mDescriptorSet}, {});
    commandBuffer.draw(mNumVertices, 1, 0, 0);
}

}}
