//
//  ComputeTests.cpp
//  Vortex2D
//

#include <gtest/gtest.h>

#include <Vortex2D/Renderer/Pipeline.h>
#include <Vortex2D/Renderer/DescriptorSet.h>

#include "Verify.h"

using namespace Vortex2D::Renderer;

extern Device* device;


TEST(ComputeTests, WriteBuffer)
{
    std::vector<float> data(100, 23.4f);
    Buffer buffer(*device, vk::BufferUsageFlagBits::eUniformBuffer, true, sizeof(float) * data.size());

    buffer.CopyFrom(data);

    CheckBuffer(data, buffer);
}

TEST(ComputeTests, BufferCopy)
{
    std::vector<float> data(100, 23.4f);
    Buffer buffer(*device, vk::BufferUsageFlagBits::eUniformBuffer, false, sizeof(float) * data.size());
    Buffer inBuffer(*device, vk::BufferUsageFlagBits::eUniformBuffer, true, sizeof(float) * data.size());
    Buffer outBuffer(*device, vk::BufferUsageFlagBits::eUniformBuffer, true, sizeof(float) * data.size());

    inBuffer.CopyFrom(data);

    device->ExecuteCommand([&](vk::CommandBuffer commandBuffer)
    {
       buffer.CopyFrom(commandBuffer, inBuffer);
    });

    device->ExecuteCommand([&](vk::CommandBuffer commandBuffer)
    {
       outBuffer.CopyFrom(commandBuffer, buffer);
       outBuffer.Barrier(commandBuffer, vk::AccessFlagBits::eHostRead);
    });

    CheckBuffer(data, outBuffer);
}

TEST(ComputeTests, BufferCompute)
{

}

TEST(ComputeTests, ImageCompute)
{
    Texture stagingTexture(*device, 50, 50, vk::Format::eR32Sfloat, true);
    Texture inTexture(*device, 50, 50, vk::Format::eR32Sfloat, false);
    Texture outTexture(*device, 50, 50, vk::Format::eR32Sfloat, false);

    std::vector<float> data(50*50,1.0f);
    stagingTexture.CopyFrom(data.data(), 4);

    device->ExecuteCommand([&](vk::CommandBuffer commandBuffer)
    {
       inTexture.CopyFrom(commandBuffer, stagingTexture);
    });

    auto shader = ShaderBuilder().File("Image.comp.spv").Create(*device);

    auto descriptorSetLayout = DescriptorSetLayoutBuilder()
            .Binding(0, vk::DescriptorType::eStorageImage, vk::ShaderStageFlagBits::eCompute, 1)
            .Binding(1, vk::DescriptorType::eStorageImage, vk::ShaderStageFlagBits::eCompute, 1)
            .Create(*device);

    auto descriptorSet = DescriptorSet(device->Handle(), descriptorSetLayout, device->DescriptorPool());

    DescriptorSetUpdater()
            .WriteDescriptorSet(descriptorSet)
            .WriteImages(0, 0, vk::DescriptorType::eStorageImage).Image({}, inTexture, vk::ImageLayout::eGeneral)
            .WriteImages(1, 0, vk::DescriptorType::eStorageImage).Image({}, outTexture, vk::ImageLayout::eGeneral)
            .Update(device->Handle());

    auto layout = PipelineLayout()
            .DescriptorSetLayout(descriptorSetLayout)
            .Create(device->Handle());

    auto pipeline = ComputePipelineBuilder()
            .Shader(shader)
            .Layout(layout)
            .Create(device->Handle());

    device->ExecuteCommand([&](vk::CommandBuffer commandBuffer)
    {
        inTexture.Barrier(commandBuffer, vk::ImageLayout::eGeneral, vk::AccessFlagBits::eShaderRead);
        outTexture.Barrier(commandBuffer, vk::ImageLayout::eGeneral, vk::AccessFlagBits::eShaderWrite);
        commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eCompute, layout, 0, {descriptorSet}, {});
        commandBuffer.bindPipeline(vk::PipelineBindPoint::eCompute, *pipeline);
        commandBuffer.dispatch(16, 16, 1);
    });

    device->Queue().waitIdle();

    device->ExecuteCommand([&](vk::CommandBuffer commandBuffer)
    {
       stagingTexture.CopyFrom(commandBuffer, outTexture);
       stagingTexture.Barrier(commandBuffer, vk::ImageLayout::eGeneral, vk::AccessFlagBits::eHostRead);
    });

    std::vector<float> doubleData(data.size(), 2.0f);
    CheckTexture(doubleData, stagingTexture, 4);
}
