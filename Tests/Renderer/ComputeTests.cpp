//
//  ComputeTests.cpp
//  Vortex
//

#include <gtest/gtest.h>
#include <algorithm>
#include <fstream>

#include <Vortex/Renderer/BindGroup.h>
#include <Vortex/Renderer/CommandBuffer.h>
#include <Vortex/Renderer/Pipeline.h>
#include <Vortex/Renderer/Timer.h>
#include <Vortex/Renderer/Work.h>
#include <Vortex/SPIRV/Reflection.h>

#include "Verify.h"

#include "vortex_tests_generated_spirv.h"

using namespace Vortex::Renderer;
using namespace Vortex::SPIRV;

extern Device* device;

TEST(ComputeTests, DISABLED_WriteBuffer)
{
  std::vector<float> data(100, 23.4f);
  Buffer<float> buffer(*device, data.size(), MemoryUsage::Cpu);

  CopyFrom(buffer, data);

  CheckBuffer(data, buffer);
}

TEST(ComputeTests, DISABLED_WriteImage)
{
  const int size = 10;
  std::vector<float> data(size * size, 23.4f);
  Texture texture(*device, size, size, Format::R32Sfloat, MemoryUsage::Cpu);

  texture.CopyFrom(data.data(), data.size());

  CheckTexture(data, texture);
}

TEST(ComputeTests, BufferCopy)
{
  std::vector<float> data(100, 23.4f);
  Buffer<float> buffer(*device, data.size());
  Buffer<float> inBuffer(*device, data.size(), MemoryUsage::CpuToGpu);
  Buffer<float> outBuffer(*device, data.size(), MemoryUsage::GpuToCpu);

  CopyFrom(inBuffer, data);

  device->Execute(
      [&](CommandEncoder& command)
      {
        buffer.CopyFrom(command, inBuffer);
        outBuffer.CopyFrom(command, buffer);
      });

  CheckBuffer(data, outBuffer);
}

TEST(ComputeTests, ImageCopy)
{
  const int size = 20;
  std::vector<float> data(size, 23.4f);

  Texture texture(*device, size, 1, Format::R32Sfloat);
  Texture inTexture(*device, size, 1, Format::R32Sfloat, MemoryUsage::CpuToGpu);
  Texture outTexture(*device, size, 1, Format::R32Sfloat, MemoryUsage::GpuToCpu);

  inTexture.CopyFrom(data.data(), data.size());

  device->Execute([&](CommandEncoder& command) {
    texture.CopyFrom(command, inTexture);
    outTexture.CopyFrom(command, texture);
  });

  CheckTexture(data, outTexture);
}

TEST(ComputeTests, ImageCopy_Large)
{
  const int width = 100;
  const int height = 10;
  std::vector<float> data(width * height, 23.4f);

  Texture texture(*device, width, height, Format::R32Sfloat);
  Texture inTexture(*device, width, height, Format::R32Sfloat, MemoryUsage::CpuToGpu);
  Texture outTexture(*device, width, height, Format::R32Sfloat, MemoryUsage::GpuToCpu);

  inTexture.CopyFrom(data.data(), data.size());

  device->Execute([&](CommandEncoder& command) {
    texture.CopyFrom(command, inTexture);
    outTexture.CopyFrom(command, texture);
  });

  CheckTexture(data, outTexture);
}

TEST(ComputeTests, UpdateVectorBuffer)
{
  int size = 3;
  Buffer<float> inBuffer(*device, size, MemoryUsage::CpuToGpu);
  Buffer<float> outBuffer(*device, size, MemoryUsage::GpuToCpu);

  std::vector<float> data(size, 1.1f);
  CopyFrom(inBuffer, data);

  device->Execute([&](CommandEncoder& command) { outBuffer.CopyFrom(command, inBuffer); });

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

TEST(ComputeTests, CopyCompute)
{
  const int size = 10;
  Buffer<float> input(*device, size, MemoryUsage::Cpu);
  Buffer<float> output(*device, size, MemoryUsage::Cpu);

  std::vector<float> data(size, 0.5f);

  CopyFrom(input, data);

  auto shader = device->CreateShaderModule(Copy_comp);

  Reflection reflection(Copy_comp);
  ShaderLayout layout(reflection);

  auto bindGroupLayout = device->CreateBindGroupLayout({layout});
  auto pipelineLayout = device->CreatePipelineLayout({layout});
  auto bindGroup = device->CreateBindGroup(bindGroupLayout, {layout}, {{input}, {output}});
  auto pipeline = device->CreateComputePipeline(shader, pipelineLayout);

  device->Execute(
      [&](CommandEncoder& command)
      {
        command.SetPipeline(PipelineBindPoint::Compute, pipeline);
        command.SetBindGroup(PipelineBindPoint::Compute, pipelineLayout, bindGroup);
        command.Dispatch(1, 1, 1);
      });

  std::vector<float> dataOut(size, 0.0f);
  CopyTo(output, dataOut);

  EXPECT_EQ(data, dataOut);
}

TEST(ComputeTests, BufferCompute)
{
  const int numParticles = 100;
  std::vector<Particle> particles(numParticles, {{1.0f, 1.0f}, {10.0f, 10.0f}});

  Buffer<Particle> buffer(*device, numParticles, MemoryUsage::Cpu);
  UniformBuffer<UBO> uboBuffer(*device, MemoryUsage::Cpu);

  UBO ubo = {0.2f, numParticles};

  CopyFrom(buffer, particles);
  CopyFrom(uboBuffer, ubo);

  auto shader = device->CreateShaderModule(Buffer_comp);

  Reflection reflection(Buffer_comp);
  ShaderLayouts layout = {reflection};
  auto bindGroupLayout = device->CreateBindGroupLayout(layout);
  auto pipelineLayout = device->CreatePipelineLayout(layout);
  auto bindGroup = device->CreateBindGroup(bindGroupLayout, layout, {{buffer}, {uboBuffer}});
  auto pipeline = device->CreateComputePipeline(shader, pipelineLayout);

  device->Execute([&](CommandEncoder& command) {
    command.SetPipeline(PipelineBindPoint::Compute, pipeline);
    command.SetBindGroup(PipelineBindPoint::Compute, pipelineLayout, bindGroup);
    command.Dispatch(1, 1, 1);
  });

  std::vector<Particle> output(numParticles);
  CopyTo(buffer, output);

  for (int i = 0; i < numParticles; i++)
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
  Texture stagingTexture(*device, 50, 50, Format::R32Sfloat, MemoryUsage::Cpu);
  Texture inTexture(*device, 50, 50, Format::R32Sfloat);
  Texture outTexture(*device, 50, 50, Format::R32Sfloat);

  std::vector<float> data(50 * 50, 1.0f);
  stagingTexture.CopyFrom(data);

  auto shader = device->CreateShaderModule(Image_comp);
  Reflection reflection(Image_comp);

  ShaderLayouts layout = {reflection};

  auto pipelineLayout = device->CreatePipelineLayout(layout);
  auto bindGroupLayout = device->CreateBindGroupLayout(layout);
  auto bindGroup = device->CreateBindGroup(bindGroupLayout, layout, {{inTexture}, {outTexture}});

  auto pipeline = device->CreateComputePipeline(shader, pipelineLayout);

  device->Execute(
      [&](CommandEncoder& command)
      {
        inTexture.CopyFrom(command, stagingTexture);
        command.SetPipeline(PipelineBindPoint::Compute, pipeline);
        command.SetBindGroup(PipelineBindPoint::Compute, pipelineLayout, bindGroup);
        command.Dispatch(16, 16, 1);
        stagingTexture.CopyFrom(command, outTexture);
      });

  std::vector<float> doubleData(data.size(), 2.0f);
  CheckTexture(doubleData, stagingTexture);
}

TEST(ComputeTests, Work)
{
  Buffer<float> buffer(*device, 16 * 16, MemoryUsage::Cpu);
  Work work(*device, ComputeSize{glm::ivec2(16)}, Work_comp, SpecConst(SpecConstValue(3, 1)));

  auto boundWork = work.Bind({buffer});

  device->Execute([&](CommandEncoder& command) { boundWork.Record(command); });

  std::vector<float> expectedOutput(16 * 16);
  for (int i = 0; i < 16; i++)
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
  Buffer<float> buffer(*device, 100 * size.x * size.y, MemoryUsage::Cpu);
  IndirectBuffer<DispatchParams> dispatchParams(*device, MemoryUsage::Cpu);

  // build work
  ComputeSize computeSize(16);
  computeSize.DomainSize.x = 16;
  computeSize.WorkSize.x = 0;
  computeSize.LocalSize.x = 8;

  Work work(*device, computeSize, WorkIndirect_comp);
  auto bound = work.Bind({buffer, dispatchParams});

  CommandBuffer cmd(*device, true);
  cmd.Record(
      [&](CommandEncoder& command)
      {
        buffer.Clear(command);
        bound.RecordIndirect(command, dispatchParams);
      });

  // Run first time
  DispatchParams params(1);
  params.workSize.x = 2;
  params.count = 16;

  CopyFrom(dispatchParams, params);

  cmd.Submit().Wait();

  // Verify
  std::vector<float> bufferData(100 * size.x * size.y);
  for (int i = 0; i < 16; i++)
  {
    bufferData[i] = 1.0f;
  }

  std::vector<float> outputData(100 * size.x * size.y, 0.0f);
  CopyTo(buffer, outputData);

  ASSERT_EQ(outputData, bufferData);

  // Update buffer and run again
  params.workSize.x = 4;
  params.count = 32;

  CopyFrom(dispatchParams, params);

  cmd.Submit().Wait();

  // Verify
  std::vector<float> bufferData2(100 * size.x * size.y, 0.0f);
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

  Texture localTexture(*device, size.x, size.y, Format::R32G32B32A32Sfloat, MemoryUsage::Cpu);
  Texture texture(*device, size.x, size.y, Format::R32G32B32A32Sfloat);

  Work work(*device, ComputeSize{size}, ImageFloat_comp);

  std::vector<glm::vec4> data(size.x * size.y, glm::vec4(2.0f, 3.0f, 0.0f, 0.0f));
  localTexture.CopyFrom(data);

  device->Execute([&](CommandEncoder& command) { texture.CopyFrom(command, localTexture); });

  auto boundWork = work.Bind({texture});

  device->Execute(
      [&](CommandEncoder& command)
      {
        boundWork.Record(command);
        texture.Barrier(
            command, ImageLayout::General, Access::Write, ImageLayout::General, Access::Read);
        localTexture.CopyFrom(command, texture);
      });

  CheckTexture<glm::vec4>(data, localTexture);
}

TEST(ComputeTests, Stencil)
{
  glm::ivec2 size(50);

  Buffer<float> input(*device, size.x * size.y, MemoryUsage::Cpu);
  Buffer<float> output(*device, size.x * size.y, MemoryUsage::Cpu);

  auto computeSize = MakeStencilComputeSize(size, 1);

  Work work(*device, computeSize, Stencil_comp);

  auto boundWork = work.Bind({input, output});

  std::vector<float> inputData(size.x * size.y, 0.0f);
  {
    float n = 1.0f;
    std::generate(inputData.begin(), inputData.end(), [&n] { return n++; });
  }

  CopyFrom(input, inputData);

  device->Execute([&](CommandEncoder& command) { boundWork.Record(command); });

  std::vector<float> expectedOutput(size.x * size.y, 0.0f);
  for (int i = 1; i < size.x - 1; i++)
  {
    for (int j = 1; j < size.y - 1; j++)
    {
      int index = i + j * size.x;
      expectedOutput[index] = inputData[index] + inputData[index + 1] + inputData[index - 1] +
                              inputData[index + size.x] + inputData[index - size.x];
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

  Buffer<float> buffer(*device, size.x * size.y, MemoryUsage::Cpu);

  ComputeSize computeSize = MakeCheckerboardComputeSize(size);

  Work work(*device, computeSize, Checkerboard_comp);

  auto boundWork = work.Bind({buffer});

  device->Execute(
      [&](CommandEncoder& command)
      {
        buffer.Clear(command);
        boundWork.PushConstant(command, 1);
        boundWork.Record(command);
      });

  std::vector<float> expectedOutput(size.x * size.y, 0.0f);
  for (int i = 0; i < size.x; i++)
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
  if (!device->HasTimer())
  {
    return;
  }

  glm::ivec2 size(500);

  Buffer<float> buffer(*device, size.x * size.y);
  Work work(*device, ComputeSize{size}, Work_comp);

  auto boundWork = work.Bind({buffer});

  CommandBuffer cmd(*device);
  cmd.Record([&](CommandEncoder& command) { boundWork.Record(command); });

  Timer timer(*device);

  timer.Start();
  cmd.Submit();
  timer.Stop();

  cmd.Wait();

  timer.Wait();
  auto time = timer.GetElapsedNs();
  ASSERT_NE(uint64_t(-1), time);
  ASSERT_NE(0, time);
  ASSERT_GT(10 * 1000 * 1000, time);  // less than 10ms

  std::cout << "Elapsed time: " << time << std::endl;
}

TEST(ComputeTests, Reflection)
{
  Reflection spirv1(Stencil_comp);

  auto descriptorTypes = spirv1.GetDescriptorTypesMap();
  BindTypeBindings expectedDescriptorTypes = {{0, BindType::StorageBuffer},
                                              {1, BindType::StorageBuffer}};

  EXPECT_EQ(expectedDescriptorTypes, descriptorTypes);

  Reflection spirv2(Image_comp);

  descriptorTypes = spirv2.GetDescriptorTypesMap();
  expectedDescriptorTypes = {{0, BindType::StorageImage}, {1, BindType::StorageImage}};

  EXPECT_EQ(expectedDescriptorTypes, descriptorTypes);

  Reflection spirv3(Checkerboard_comp);
  EXPECT_EQ(12, spirv3.GetPushConstantsSize());
}

TEST(ComputeTests, Cache)
{
  auto shader1 = device->CreateShaderModule(Buffer_comp);
  Reflection reflection1(Buffer_comp);

  ShaderLayouts layout1 = {reflection1};

  Handle::PipelineLayout pipelineLayout1 = device->CreatePipelineLayout(layout1);
  Handle::Pipeline pipeline1 = device->CreateComputePipeline(shader1, pipelineLayout1);

  auto shader2 = device->CreateShaderModule(Image_comp);
  Reflection reflection2(Image_comp);

  ShaderLayouts layout2 = {reflection2};
  Handle::PipelineLayout pipelineLayout2 = device->CreatePipelineLayout(layout2);

  Handle::Pipeline pipeline2 = device->CreateComputePipeline(shader2, pipelineLayout2);

  EXPECT_NE(shader1, shader2);
  EXPECT_NE(pipelineLayout1, pipelineLayout2);
  EXPECT_NE(pipeline1, pipeline2);

  EXPECT_EQ(shader1, device->CreateShaderModule(Buffer_comp));
  EXPECT_EQ(pipelineLayout1, device->CreatePipelineLayout(layout1));
  EXPECT_EQ(pipeline1, device->CreateComputePipeline(shader1, pipelineLayout1));

  EXPECT_EQ(shader2, device->CreateShaderModule(Image_comp));
  EXPECT_EQ(pipelineLayout2, device->CreatePipelineLayout(layout2));
  EXPECT_EQ(pipeline2, device->CreateComputePipeline(shader2, pipelineLayout2));
}
