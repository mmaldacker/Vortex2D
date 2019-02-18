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

#include "vortex2d_tests_generated_spirv.h"

using namespace Vortex2D::Renderer;
using namespace Vortex2D::SPIRV;

extern Device* device;

TEST(ComputeTests, WriteBuffer)
{
    std::vector<float> data(100, 23.4f);
    Buffer<float> buffer(*device, data.size(), VMA_MEMORY_USAGE_CPU_ONLY);

    CopyFrom(buffer, data);

    CheckBuffer(data, buffer);
}

TEST(ComputeTests, BufferCopy)
{
    std::vector<float> data(100, 23.4f);
    Buffer<float> buffer(*device, data.size());
    Buffer<float> inBuffer(*device, data.size(), VMA_MEMORY_USAGE_CPU_ONLY);
    Buffer<float> outBuffer(*device, data.size(), VMA_MEMORY_USAGE_CPU_ONLY);

    CopyFrom(inBuffer, data);

    device->Execute([&](vk::CommandBuffer commandBuffer)
    {
       buffer.CopyFrom(commandBuffer, inBuffer);
       outBuffer.CopyFrom(commandBuffer, buffer);
    });

    CheckBuffer(data, outBuffer);
}

TEST(ComputeTests, UpdateVectorBuffer)
{
    int size = 3;
    Buffer<float> inBuffer(*device, size, VMA_MEMORY_USAGE_CPU_TO_GPU);
    Buffer<float> outBuffer(*device, size, VMA_MEMORY_USAGE_GPU_TO_CPU);

    std::vector<float> data(size, 1.1f);
    CopyFrom(inBuffer, data);

    device->Execute([&](vk::CommandBuffer commandBuffer)
    {
        outBuffer.CopyFrom(commandBuffer, inBuffer);
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

    Buffer<Particle> buffer(*device, 100, VMA_MEMORY_USAGE_CPU_ONLY);
    UniformBuffer<UBO> uboBuffer(*device, VMA_MEMORY_USAGE_CPU_ONLY);

    UBO ubo = {0.2f, 100};

    CopyFrom(buffer, particles);
    CopyFrom(uboBuffer, ubo);

    auto shader = device->GetShaderModule(Buffer_comp);
    Reflection reflection(Buffer_comp);

    PipelineLayout layout = {{reflection}};
    DescriptorSet descriptorSet = device->GetLayoutManager().MakeDescriptorSet(layout);
    Bind(*device, descriptorSet, layout, {{buffer}, {uboBuffer}});

    auto pipeline = MakeComputePipeline(device->Handle(), shader, descriptorSet.pipelineLayout);

    device->Execute([&](vk::CommandBuffer commandBuffer)
    {
        commandBuffer.bindPipeline(vk::PipelineBindPoint::eCompute, *pipeline);
        commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eCompute, descriptorSet.pipelineLayout, 0, {*descriptorSet.descriptorSet}, {});
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
    Texture stagingTexture(*device, 50, 50, vk::Format::eR32Sfloat, VMA_MEMORY_USAGE_CPU_ONLY);
    Texture inTexture(*device, 50, 50, vk::Format::eR32Sfloat);
    Texture outTexture(*device, 50, 50, vk::Format::eR32Sfloat);

    std::vector<float> data(50*50,1.0f);
    stagingTexture.CopyFrom(data);

    auto shader = device->GetShaderModule(Image_comp);
    Reflection reflection(Image_comp);

    PipelineLayout layout = {{reflection}};
    DescriptorSet descriptorSet = device->GetLayoutManager().MakeDescriptorSet(layout);
    Bind(*device, descriptorSet, layout, {{inTexture}, {outTexture}});

    auto pipeline = MakeComputePipeline(device->Handle(), shader, descriptorSet.pipelineLayout);

    device->Execute([&](vk::CommandBuffer commandBuffer)
    {
        inTexture.CopyFrom(commandBuffer, stagingTexture);
        commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eCompute, descriptorSet.pipelineLayout, 0, {*descriptorSet.descriptorSet}, {});
        commandBuffer.bindPipeline(vk::PipelineBindPoint::eCompute, *pipeline);
        commandBuffer.dispatch(16, 16, 1);
        stagingTexture.CopyFrom(commandBuffer, outTexture);
    });

    std::vector<float> doubleData(data.size(), 2.0f);
    CheckTexture(doubleData, stagingTexture);
}

TEST(ComputeTests, Work)
{
    Buffer<float> buffer(*device, 16*16, VMA_MEMORY_USAGE_CPU_ONLY);
    Work work(*device, glm::ivec2(16), Work_comp, SpecConst(SpecConstValue(3, 1)));

    auto boundWork = work.Bind({buffer});

    device->Execute([&](vk::CommandBuffer commandBuffer)
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
    Buffer<float> buffer(*device, 100*size.x*size.y, VMA_MEMORY_USAGE_CPU_ONLY);
    IndirectBuffer<DispatchParams> dispatchParams(*device, VMA_MEMORY_USAGE_CPU_ONLY);

    // build work
    ComputeSize computeSize(16);
    computeSize.DomainSize.x = 16;
    computeSize.WorkSize.x = 0;
    computeSize.LocalSize.x = 8;

    Work work(*device, computeSize, WorkIndirect_comp);
    auto bound = work.Bind({buffer, dispatchParams});

    CommandBuffer cmd(*device, true);
    cmd.Record([&](vk::CommandBuffer commandBuffer)
    {
        bound.RecordIndirect(commandBuffer, dispatchParams);
    });

    // Run first time
    DispatchParams params(1);
    params.workSize.x = 2;
    params.count = 16;

    CopyFrom(dispatchParams, params);

    cmd.Submit().Wait();

    // Verify
    std::vector<float> bufferData(100*size.x*size.y);
    for (int i = 0; i < 16; i++)
    {
        bufferData[i] = 1.0f;
    }

    std::vector<float> outputData(100*size.x*size.y, 0.0f);
    CopyTo(buffer, outputData);

    ASSERT_EQ(outputData, bufferData);

    // Update buffer and run again
    params.workSize.x = 4;
    params.count = 32;

    CopyFrom(dispatchParams, params);

    cmd.Submit().Wait();

    // Verify
    std::vector<float> bufferData2(100*size.x*size.y, 0.0f);
    for (int i = 0; i < 32; i++)
    {
        bufferData2[i] = 1.0f;
    }

    CopyTo(buffer, outputData);

    ASSERT_NE(outputData, bufferData);
    ASSERT_EQ(outputData, bufferData2);
}

TEST(ComputeTests, FloatImage)
{
    glm::ivec2 size(20);

    Texture localTexture(*device, size.x, size.y, vk::Format::eR32G32B32A32Sfloat, VMA_MEMORY_USAGE_CPU_ONLY);
    Texture texture(*device, size.x, size.y, vk::Format::eR32G32B32A32Sfloat);

    Work work(*device, size, ImageFloat_comp);

    std::vector<glm::vec4> data(size.x*size.y, glm::vec4(2.0f, 3.0f, 0.0f, 0.0f));
    localTexture.CopyFrom(data);

    device->Execute([&](vk::CommandBuffer commandBuffer)
    {
       texture.CopyFrom(commandBuffer, localTexture);
    });

    auto boundWork = work.Bind({texture});

    device->Execute([&](vk::CommandBuffer commandBuffer)
    {
       boundWork.Record(commandBuffer);
       texture.Barrier(commandBuffer,
                       vk::ImageLayout::eGeneral, vk::AccessFlagBits::eShaderWrite,
                       vk::ImageLayout::eGeneral, vk::AccessFlagBits::eShaderRead);
       localTexture.CopyFrom(commandBuffer, texture);
    });

    CheckTexture<glm::vec4>(data, localTexture);
}

TEST(ComputeTests, Stencil)
{
    glm::ivec2 size(50);

    Buffer<float> input(*device, size.x*size.y, VMA_MEMORY_USAGE_CPU_ONLY);
    Buffer<float> output(*device, size.x*size.y, VMA_MEMORY_USAGE_CPU_ONLY);

    auto computeSize = MakeStencilComputeSize(size, 1);

    Work work(*device, computeSize, Stencil_comp);

    auto boundWork = work.Bind({input, output});

    std::vector<float> inputData(size.x*size.y, 0.0f);
    {
        float n = 1.0f;
        std::generate(inputData.begin(), inputData.end(), [&n]{ return n++; });
    }

    CopyFrom(input, inputData);

    device->Execute([&](vk::CommandBuffer commandBuffer)
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

    Buffer<float> buffer(*device, size.x*size.y, VMA_MEMORY_USAGE_CPU_ONLY);

    ComputeSize computeSize = MakeCheckerboardComputeSize(size);

    Work work(*device, computeSize, Checkerboard_comp);

    auto boundWork = work.Bind({buffer});

    device->Execute([&](vk::CommandBuffer commandBuffer)
    {
        boundWork.PushConstant(commandBuffer, 1);
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
    auto properties = device->GetPhysicalDevice().getProperties();
    if (!properties.limits.timestampComputeAndGraphics)
    {
        return;
    }

    glm::ivec2 size(500);

    Buffer<float> buffer(*device, size.x*size.y);
    Work work(*device, size, Work_comp);

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

    timer.Wait();
    auto time = timer.GetElapsedNs();
    ASSERT_NE(uint64_t(-1), time);
    ASSERT_NE(0, time);
    ASSERT_GT(1000 * 1000, time); // less than 1ms

    std::cout << "Elapsed time: " << time << std::endl;
}

TEST(ComputeTests, Reflection)
{
  Reflection spirv1(Stencil_comp);

  auto descriptorTypes = spirv1.GetDescriptorTypesMap();
  DescriptorTypeBindings expectedDescriptorTypes = {{0, vk::DescriptorType::eStorageBuffer}, {1, vk::DescriptorType::eStorageBuffer}};

  EXPECT_EQ(expectedDescriptorTypes, descriptorTypes);

  Reflection spirv2(Image_comp);

  descriptorTypes = spirv2.GetDescriptorTypesMap();
  expectedDescriptorTypes = {{0, vk::DescriptorType::eStorageImage}, {1, vk::DescriptorType::eStorageImage}};

  EXPECT_EQ(expectedDescriptorTypes, descriptorTypes);

  Reflection spirv3(Checkerboard_comp);
  EXPECT_EQ(12, spirv3.GetPushConstantsSize());
}
