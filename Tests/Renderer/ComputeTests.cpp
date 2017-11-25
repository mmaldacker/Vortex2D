//
//  ComputeTests.cpp
//  Vortex2D
//

#include <gtest/gtest.h>
#include <fstream>

#include <Vortex2D/Renderer/Pipeline.h>
#include <Vortex2D/Renderer/DescriptorSet.h>
#include <Vortex2D/Renderer/Work.h>
#include <Vortex2D/Renderer/CommandBuffer.h>
#include <Vortex2D/Renderer/Timer.h>
#include <Vortex2D/Renderer/DescriptorSet.h>
#include <Vortex2D/SPIRV/Reflection.h>

#include "Verify.h"

using namespace Vortex2D::Renderer;
using namespace Vortex2D::SPIRV;

extern Device* device;

TEST(ComputeTests, WriteBuffer)
{
    std::vector<float> data(100, 23.4f);
    Buffer<float> buffer(*device, data.size(), true);

    CopyFrom(buffer, data);

    CheckBuffer(data, buffer);
}

TEST(ComputeTests, BufferCopy)
{
    std::vector<float> data(100, 23.4f);
    Buffer<float> buffer(*device, data.size());
    Buffer<float> inBuffer(*device, data.size(), true);
    Buffer<float> outBuffer(*device, data.size(), true);

    CopyFrom(inBuffer, data);

    ExecuteCommand(*device, [&](vk::CommandBuffer commandBuffer)
    {
       buffer.CopyFrom(commandBuffer, inBuffer);
       outBuffer.CopyFrom(commandBuffer, buffer);
    });

    CheckBuffer(data, outBuffer);
}

TEST(ComputeTests, UpdateVectorBuffer)
{
    int size = 3;
    UpdateStorageBuffer<float> inBuffer(*device, size);
    UpdateStorageBuffer<float> outBuffer(*device, size);

    std::vector<float> data(size, 1.1f);
    CopyFrom(inBuffer, data);

    ExecuteCommand(*device, [&](vk::CommandBuffer commandBuffer)
    {
        inBuffer.Upload(commandBuffer);
        outBuffer.CopyFrom(commandBuffer, inBuffer);
        outBuffer.Download(commandBuffer);
    });

    std::vector<float> outData(size);
    CopyTo(outBuffer, outData);

    EXPECT_EQ(outData, data);
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

    Buffer<Particle> buffer(*device, 100, true);
    UniformBuffer<UBO> uboBuffer(*device, true);

    UBO ubo = {0.2f, 100};

    CopyFrom(buffer, particles);
    CopyFrom(uboBuffer, ubo);

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
    CopyTo(buffer, output);

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
            .WriteImages(0, 0, vk::DescriptorType::eStorageImage).Image({}, inTexture.GetView(), vk::ImageLayout::eGeneral)
            .WriteImages(1, 0, vk::DescriptorType::eStorageImage).Image({}, outTexture.GetView(), vk::ImageLayout::eGeneral)
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
    Buffer<float> buffer(*device, 16*16, true);
    Work work(*device, glm::ivec2(16), "Work.comp.spv");

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

TEST(ComputeTests, WorkIndirect)
{
    glm::ivec2 size(16, 1);

    // create bigger buffer than size to check the indirect buffer is correct
    Buffer<float> buffer(*device, 100*size.x*size.y, true);
    Buffer<DispatchParams> dispatchParams(*device, 1, true);

    DispatchParams params(1);
    params.workSize.x = 2;
    params.count = 16;

    CopyFrom(dispatchParams, params);

    ComputeSize computeSize(16);
    computeSize.DomainSize.x = 16;
    computeSize.WorkSize.x = 2;
    computeSize.LocalSize.x = 8;

    Work work(*device, computeSize, "WorkIndirect.comp.spv");
    auto bound = work.Bind({buffer, dispatchParams});

    // Run first time
    ExecuteCommand(*device, [&](vk::CommandBuffer commandBuffer)
    {
       bound.RecordIndirect(commandBuffer, dispatchParams);
    });

    std::vector<float> bufferData(100*size.x*size.y);
    for (int i = 0; i < 16; i++)
    {
        bufferData[i] = 1.0f;
    }

    std::vector<float> outputData(100*size.x*size.y);
    CopyTo(buffer, outputData);

    ASSERT_EQ(outputData, bufferData);

    // Update buffer and run again
    params.workSize.x = 1;
    params.count = 8;

    //CopyFrom(dispatchParams, params);

    ExecuteCommand(*device, [&](vk::CommandBuffer commandBuffer)
    {
       buffer.Clear(commandBuffer);
       bound.RecordIndirect(commandBuffer, dispatchParams);
    });

    std::vector<float> bufferData2(100*size.x*size.y);
    for (int i = 0; i < 8; i++)
    {
        bufferData2[i] = 1.0f;
    }

    CopyTo(buffer, outputData);

    ASSERT_EQ(outputData, bufferData2);
}

TEST(ComputeTests, Stencil)
{
    glm::ivec2 size(50);

    Buffer<float> input(*device, size.x*size.y, true);
    Buffer<float> output(*device, size.x*size.y, true);

    auto computeSize = MakeStencilComputeSize(size, 1);

    Work work(*device, computeSize, "Stencil.comp.spv");

    auto boundWork = work.Bind({input, output});

    std::vector<float> inputData(size.x*size.y, 0.0f);
    {
        float n = 1.0f;
        std::generate(inputData.begin(), inputData.end(), [&n]{ return n++; });
    }

    CopyFrom(input, inputData);

    ExecuteCommand(*device, [&](vk::CommandBuffer commandBuffer)
    {
        boundWork.Record(commandBuffer);
    });

    std::vector<float> expectedOutput(size.x*size.y, 0.0f);
    for (int i = 1; i < size.x - 1; i++)
    {
      for (int j = 1; j < size.y - 1; j++)
      {
        int index = i + j * size.x;
        expectedOutput[index] =
            inputData[index] +
            inputData[index + 1] +
            inputData[index - 1] +
            inputData[index + size.x] +
            inputData[index - size.x];
      }
    }

    std::vector<float> bufferOutput(size.x * size.y);
    CopyTo(output, bufferOutput);

    for (int i = 1; i < size.x - 1; i++)
    {
      for (int j = 1; j < size.y - 1; j++)
      {
        int index = i + j * size.x;

        float expectedValue = expectedOutput[index];
        float value = bufferOutput[index];
        EXPECT_EQ(expectedValue, value) << "Value not equal at " << index;
      }
    }
}

TEST(ComputeTests, Checkerboard)
{
    glm::ivec2 size(50);

    Buffer<float> buffer(*device, size.x*size.y, true);

    ComputeSize computeSize = MakeCheckerboardComputeSize(size);

    Work work(*device, computeSize, "Checkerboard.comp.spv");

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

    Buffer<float> buffer(*device, size.x*size.y);
    Work work(*device, size, "Work.comp.spv");

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

    Buffer<float> flop(*device, size.x*size.y);
    Buffer<float> flip(*device, size.x*size.y);

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

std::vector<uint32_t> ReadFile(const std::string& filename)
{
    std::ifstream is(filename, std::ios::binary | std::ios::in | std::ios::ate);
    std::vector<uint32_t> content;

    size_t size = is.tellg();
    is.seekg(0, std::ios::beg);
    content.resize(size / 4);
    is.read(reinterpret_cast<char*>(content.data()), size);
    is.close();

    return content;
}

TEST(ComputeTests, Reflection)
{
  Reflection spirv1(ReadFile("Stencil.comp.spv"));

  auto descriptorTypes = spirv1.GetDescriptorTypesMap();
  DescriptorTypeBindings expectedDescriptorTypes = {{0, vk::DescriptorType::eStorageBuffer}, {1, vk::DescriptorType::eStorageBuffer}};

  EXPECT_EQ(expectedDescriptorTypes, descriptorTypes);

  Reflection spirv2(ReadFile("Image.comp.spv"));

  descriptorTypes = spirv2.GetDescriptorTypesMap();
  expectedDescriptorTypes = {{0, vk::DescriptorType::eStorageImage}, {1, vk::DescriptorType::eStorageImage}};

  EXPECT_EQ(expectedDescriptorTypes, descriptorTypes);

  Reflection spirv3(ReadFile("Redistance.comp.spv"));

  descriptorTypes = spirv3.GetDescriptorTypesMap();
  expectedDescriptorTypes = {{0, vk::DescriptorType::eCombinedImageSampler}, {1, vk::DescriptorType::eCombinedImageSampler}, {2, vk::DescriptorType::eStorageImage}};

  EXPECT_EQ(expectedDescriptorTypes, descriptorTypes);
  EXPECT_EQ(12, spirv3.GetPushConstantsSize());
}
