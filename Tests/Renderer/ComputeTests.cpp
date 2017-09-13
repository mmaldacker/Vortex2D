//
//  ComputeTests.cpp
//  Vortex2D
//

#include <gtest/gtest.h>

#include <Vortex2D/Renderer/Pipeline.h>
#include <Vortex2D/Renderer/DescriptorSet.h>
#include <Vortex2D/Renderer/Work.h>
#include <Vortex2D/Renderer/CommandBuffer.h>
#include <Vortex2D/Renderer/Timer.h>

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

    ExecuteCommand(*device, [&](vk::CommandBuffer commandBuffer)
    {
       buffer.CopyFrom(commandBuffer, inBuffer);
       outBuffer.CopyFrom(commandBuffer, buffer);
    });

    CheckBuffer(data, outBuffer);
}

TEST(ComputeTests, PushConstantsSize)
{
    auto size = PushConstantsSize<int, float, uint8_t>();
    ASSERT_EQ(9, size);
}

struct Particle
{
    alignas(8) glm::vec2 position;
    alignas(8) glm::vec2 velocity;
};

struct UBO
{
    alignas(4) float deltaT;
    alignas(4) int particleCount;
};

TEST(ComputeTests, BufferCompute)
{
    std::vector<Particle> particles(100, {{1.0f, 1.0f}, {10.0f, 10.0f}});

    Buffer buffer(*device, vk::BufferUsageFlagBits::eStorageBuffer, true, sizeof(Particle) * 100);
    Buffer uboBuffer(*device, vk::BufferUsageFlagBits::eUniformBuffer, true, sizeof(UBO));

    UBO ubo = {0.2f, 100};

    buffer.CopyFrom(particles);
    uboBuffer.CopyFrom(ubo);

    auto shader = device->GetShaderModule("Buffer.comp.spv");

    auto descriptorLayout = DescriptorSetLayoutBuilder()
            .Binding(0, vk::DescriptorType::eStorageBuffer, vk::ShaderStageFlagBits::eCompute, 1)
            .Binding(1, vk::DescriptorType::eUniformBuffer, vk::ShaderStageFlagBits::eCompute, 1)
            .Create(*device);

    auto descriptorSet = MakeDescriptorSet(*device, descriptorLayout);

    DescriptorSetUpdater(*descriptorSet)
            .WriteBuffers(0, 0, vk::DescriptorType::eStorageBuffer).Buffer(buffer)
            .WriteBuffers(1, 0, vk::DescriptorType::eUniformBuffer).Buffer(uboBuffer)
            .Update(device->Handle());

    auto layout = PipelineLayoutBuilder()
            .DescriptorSetLayout(descriptorLayout)
            .Create(device->Handle());

    auto pipeline = MakeComputePipeline(device->Handle(), shader, *layout);

    ExecuteCommand(*device, [&](vk::CommandBuffer commandBuffer)
    {
        commandBuffer.bindPipeline(vk::PipelineBindPoint::eCompute, *pipeline);
        commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eCompute, *layout, 0, {*descriptorSet}, {});
        commandBuffer.dispatch(1, 1, 1);
    });

    std::vector<Particle> output(100);
    buffer.CopyTo(output);

    for (int i = 0; i < 100; i++)
    {
        auto particle = output[i];
        Particle expectedParticle = {{3.0f, 3.0f}, {10.0f, 10.0f}};

        EXPECT_FLOAT_EQ(expectedParticle.position.x, particle.position.x) << "Value not equal at " << i;
        EXPECT_FLOAT_EQ(expectedParticle.position.x, particle.position.y) << "Value not equal at " << i;
        EXPECT_FLOAT_EQ(expectedParticle.velocity.x, particle.velocity.x) << "Value not equal at " << i;
        EXPECT_FLOAT_EQ(expectedParticle.velocity.y, particle.velocity.y) << "Value not equal at " << i;
    }
}

TEST(ComputeTests, ImageCompute)
{
    Texture stagingTexture(*device, 50, 50, vk::Format::eR32Sfloat, true);
    Texture inTexture(*device, 50, 50, vk::Format::eR32Sfloat, false);
    Texture outTexture(*device, 50, 50, vk::Format::eR32Sfloat, false);

    std::vector<float> data(50*50,1.0f);
    stagingTexture.CopyFrom(data);

    auto shader = device->GetShaderModule("Image.comp.spv");

    auto descriptorSetLayout = DescriptorSetLayoutBuilder()
            .Binding(0, vk::DescriptorType::eStorageImage, vk::ShaderStageFlagBits::eCompute, 1)
            .Binding(1, vk::DescriptorType::eStorageImage, vk::ShaderStageFlagBits::eCompute, 1)
            .Create(*device);

    auto descriptorSet = MakeDescriptorSet(*device, descriptorSetLayout);

    DescriptorSetUpdater(*descriptorSet)
            .WriteImages(0, 0, vk::DescriptorType::eStorageImage).Image({}, inTexture, vk::ImageLayout::eGeneral)
            .WriteImages(1, 0, vk::DescriptorType::eStorageImage).Image({}, outTexture, vk::ImageLayout::eGeneral)
            .Update(device->Handle());

    auto layout = PipelineLayoutBuilder()
            .DescriptorSetLayout(descriptorSetLayout)
            .Create(device->Handle());

    auto pipeline = MakeComputePipeline(device->Handle(), shader, *layout);

    ExecuteCommand(*device, [&](vk::CommandBuffer commandBuffer)
    {
        inTexture.CopyFrom(commandBuffer, stagingTexture);
        commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eCompute, *layout, 0, {*descriptorSet}, {});
        commandBuffer.bindPipeline(vk::PipelineBindPoint::eCompute, *pipeline);
        commandBuffer.dispatch(16, 16, 1);
        stagingTexture.CopyFrom(commandBuffer, outTexture);
    });

    std::vector<float> doubleData(data.size(), 2.0f);
    CheckTexture(doubleData, stagingTexture);
}

TEST(ComputeTests, Work)
{
    Buffer buffer(*device, vk::BufferUsageFlagBits::eStorageBuffer, true, sizeof(float)*16*16);
    Work work(*device, glm::ivec2(16), "Work.comp.spv", {vk::DescriptorType::eStorageBuffer});

    auto boundWork = work.Bind({buffer});

    ExecuteCommand(*device, [&](vk::CommandBuffer commandBuffer)
    {
        boundWork.Record(commandBuffer);
    });

    std::vector<float> expectedOutput(16*16);
    for (int i = 0; i < 16; i ++)
    {
        for (int j = 0; j < 16; j++)
        {
            if ((i + j) % 2 == 0)
            {
                expectedOutput[i + j * 16] = 1;
            }
            else
            {
                expectedOutput[i + j * 16] = 0;
            }
        }
    }

    CheckBuffer(expectedOutput, buffer);
}

TEST(ComputeTests, Stencil)
{
    glm::ivec2 size(50);

    Buffer input(*device, vk::BufferUsageFlagBits::eStorageBuffer, true, sizeof(float)*size.x*size.y);
    Buffer output(*device, vk::BufferUsageFlagBits::eStorageBuffer, true, sizeof(float)*size.x*size.y);

    auto computeSize = MakeStencilComputeSize(size, 1);

    Work work(*device, computeSize, "Stencil.comp.spv", {vk::DescriptorType::eStorageBuffer, vk::DescriptorType::eStorageBuffer});

    auto boundWork = work.Bind({input, output});

    std::vector<float> inputData(size.x*size.y, 1.0f);

    input.CopyFrom(inputData);

    ExecuteCommand(*device, [&](vk::CommandBuffer commandBuffer)
    {
        boundWork.Record(commandBuffer);
    });

    std::vector<float> expectedOutput(size.x*size.y, 5.0f);
    CheckBuffer(expectedOutput, output);
}

TEST(ComputeTests, Checkerboard)
{
    glm::ivec2 size(50);

    Buffer buffer(*device, vk::BufferUsageFlagBits::eStorageBuffer, true, sizeof(float)*size.x*size.y);

    ComputeSize computeSize = MakeCheckerboardComputeSize(size);

    Work work(*device, computeSize, "Checkerboard.comp.spv", {vk::DescriptorType::eStorageBuffer}, PushConstantsSize<int>());

    auto boundWork = work.Bind({buffer});

    ExecuteCommand(*device, [&](vk::CommandBuffer commandBuffer)
    {
        boundWork.PushConstant(commandBuffer, 8, 1);
        boundWork.Record(commandBuffer);
    });

    std::vector<float> expectedOutput(size.x*size.y, 0.0f);
    for (int i = 0; i < size.x; i ++)
    {
        for (int j = 0; j < size.y; j++)
        {
            if ((i + j) % 2 == 1)
            {
                expectedOutput[i + j * size.x] = 1.0f;
            }
        }
    }

    CheckBuffer(expectedOutput, buffer);
}

TEST(ComputeTests, Timer)
{
    glm::ivec2 size(500);

    Buffer buffer(*device, vk::BufferUsageFlagBits::eStorageBuffer, false, sizeof(float)*size.x*size.y);
    Work work(*device, size, "Work.comp.spv", {vk::DescriptorType::eStorageBuffer});

    auto boundWork = work.Bind({buffer});

    CommandBuffer cmd(*device);
    cmd.Record([&](vk::CommandBuffer commandBuffer)
    {
        boundWork.Record(commandBuffer);
    });

    Timer timer(*device);

    timer.Start();
    cmd.Submit();
    timer.Stop();

    cmd.Wait();

    auto time = timer.GetElapsedNs();
    ASSERT_NE(uint64_t(-1), time);
    ASSERT_NE(0, time);

    std::cout << "Elapsed time: " << time << std::endl;
}

TEST(ComputeTests, Statistics)
{
    glm::ivec2 size(500);

    Statistics stats(*device);

    Buffer flop(*device, vk::BufferUsageFlagBits::eStorageBuffer, false, sizeof(float)*size.x*size.y);
    Buffer flip(*device, vk::BufferUsageFlagBits::eStorageBuffer, false, sizeof(float)*size.x*size.y);

    ExecuteCommand(*device, [&](vk::CommandBuffer commandBuffer)
    {
        for (int i = 0; i < 10; i++)
        {
            stats.Start(commandBuffer);
            flop.CopyFrom(commandBuffer, flip);
            stats.Tick(commandBuffer, "Copy1");
            flip.CopyFrom(commandBuffer, flop);
            stats.Tick(commandBuffer, "Copy2");
            flop.CopyFrom(commandBuffer, flip);
            stats.End(commandBuffer, "Copy3");
        }
    });

    auto timestamps = stats.GetTimestamps();
    ASSERT_EQ(3, timestamps.size());
    ASSERT_EQ("Copy1", timestamps[0].first);
    ASSERT_EQ("Copy2", timestamps[1].first);
    ASSERT_EQ("Copy3", timestamps[2].first);

    std::cout << "Timestamp " << timestamps[0].first << ": " << timestamps[0].second << std::endl;
    std::cout << "Timestamp " << timestamps[1].first << ": " << timestamps[1].second << std::endl;
    std::cout << "Timestamp " << timestamps[2].first << ": " << timestamps[2].second << std::endl;
}
