//
//  ParticleTests.cpp
//  Vortex2D
//

#include "Verify.h"

#include <random>

#include <Vortex2D/Renderer/Shapes.h>
#include <Vortex2D/Engine/PrefixScan.h>
#include <Vortex2D/Engine/Particles.h>

using namespace Vortex2D::Renderer;
using namespace Vortex2D::Fluid;

extern Device* device;

std::vector<int> GenerateInput(int size)
{
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<int> dist(0, 4);

    std::vector<int> input(size);
    for (int i = 0; i < size; i++)
    {
        input[i] = dist(gen);
    }

    return input;
}

std::vector<int> CalculatePrefixScan(const std::vector<int>& input)
{
    std::vector<int> output(input.size());

    for(unsigned i = 1; i < input.size(); ++i)
    {
        output[i] = input[i-1] + output[i-1];
    }

    return output;
}

void PrintPrefixBuffer(const glm::ivec2& size, Buffer& buffer)
{
    int n = size.x * size.y;
    std::vector<int> pixels(n);
    buffer.CopyTo(pixels);

    for (int i = 0; i < n; i++)
    {
        int value = pixels[i];
        std::cout << "(" << value << ")";
    }
    std::cout << std::endl;
}

void PrintPrefixVector(const std::vector<int>& data)
{
    for (auto value: data)
    {
        std::cout << "(" << value << ")";
    }
    std::cout << std::endl;
}

TEST(ParticleTests, PrefixScan)
{
    glm::ivec2 size(20, 20);

    PrefixScan prefixScan(*device, size);

    Buffer input(*device, vk::BufferUsageFlagBits::eStorageBuffer, true, size.x*size.y*sizeof(int));
    Buffer output(*device, vk::BufferUsageFlagBits::eStorageBuffer, true, size.x*size.y*sizeof(int));
    Buffer dispatchParams(*device, vk::BufferUsageFlagBits::eStorageBuffer, true, sizeof(DispatchParams));

    std::vector<int> inputData = GenerateInput(size.x*size.y);
    input.CopyFrom(inputData);

    auto bound = prefixScan.Bind(input, output, dispatchParams);

    ExecuteCommand(*device, [&](vk::CommandBuffer commandBuffer)
    {
        bound.Record(commandBuffer);
    });

    auto outputData = CalculatePrefixScan(inputData);
    CheckBuffer(outputData, output);

    DispatchParams params(0);
    dispatchParams.CopyTo(params);

    int total = outputData.back() + inputData.back();
    ASSERT_EQ(params.count, total);
    ASSERT_EQ(params.workSize.x, std::ceil((float)total / 256));
    ASSERT_EQ(params.workSize.y, 1);
    ASSERT_EQ(params.workSize.z, 1);
}

TEST(ParticleTests, PrefixScanBig)
{
    glm::ivec2 size(100, 100);

    PrefixScan prefixScan(*device, size);

    Buffer input(*device, vk::BufferUsageFlagBits::eStorageBuffer, true, size.x*size.y*sizeof(int));
    Buffer output(*device, vk::BufferUsageFlagBits::eStorageBuffer, true, size.x*size.y*sizeof(int));
    Buffer dispatchParams(*device, vk::BufferUsageFlagBits::eStorageBuffer, true, sizeof(DispatchParams));

    std::vector<int> inputData = GenerateInput(size.x*size.y);
    input.CopyFrom(inputData);

    auto bound = prefixScan.Bind(input, output, dispatchParams);

    ExecuteCommand(*device, [&](vk::CommandBuffer commandBuffer)
    {
        bound.Record(commandBuffer);
    });

    auto outputData = CalculatePrefixScan(inputData);
    CheckBuffer(outputData, output);

    DispatchParams params(0);
    dispatchParams.CopyTo(params);

    int total = outputData.back() + inputData.back();
    ASSERT_EQ(params.count, total); ASSERT_EQ(params.workSize.x, std::ceil((float)total / 256));
    ASSERT_EQ(params.workSize.y, 1);
    ASSERT_EQ(params.workSize.z, 1);
}

TEST(ParticleTests, ParticleCount)
{
    glm::ivec2 size(20, 20);

    std::vector<Particle> particlesData(size.x*size.y*8);
    particlesData[0].Position = glm::vec2(3.4f, 2.3f);
    particlesData[1].Position = glm::vec2(3.5f, 2.4f);
    particlesData[2].Position = glm::vec2(5.4f, 6.7f);
    int particleCount = 3;

    Buffer particles(*device, vk::BufferUsageFlagBits::eStorageBuffer, true, 8*size.x*size.y*sizeof(Particle));
    particles.CopyFrom(particlesData);

    Particles particle(*device, size, particles, {particleCount});

    particle.Count();
    device->Handle().waitIdle();

    Texture localCount(*device, size.x, size.y, vk::Format::eR32Sint, true);
    ExecuteCommand(*device, [&](vk::CommandBuffer commandBuffer)
    {
        localCount.CopyFrom(commandBuffer, particle);
    });

    std::vector<int> outData(size.x*size.y);
    localCount.CopyTo(outData);

    glm::ivec2 pos1(3, 2);
    glm::ivec2 pos2(5, 6);
    ASSERT_EQ(2, outData[pos1.x + pos1.y * size.x]);
    ASSERT_EQ(1, outData[pos2.x + pos2.y * size.x]);
}

TEST(ParticleTests, ParticleDelete)
{
    glm::ivec2 size(20, 20);

    std::vector<Particle> particlesData(size.x*size.y*8);
    particlesData[0].Position = glm::vec2(3.4f, 2.3f);
    particlesData[1].Position = glm::vec2(13.4f, 16.7f);
    particlesData[2].Position = glm::vec2(3.5f, 2.4f);
    int particleCount = 3;

    Buffer particles(*device, vk::BufferUsageFlagBits::eStorageBuffer, true, 8*size.x*size.y*sizeof(Particle));
    particles.CopyFrom(particlesData);

    Particles particle(*device, size, particles, {particleCount});

    // Count particles
    particle.Count();
    device->Handle().waitIdle();

    // Delete some particles
    IntRectangle rect(*device, {10, 10}, glm::ivec4(0));
    rect.Position = glm::vec2(10.0f, 10.0f);
    rect.Initialize(particle);
    rect.Update(particle.Orth, {});

    particle.Record([&](vk::CommandBuffer commandBuffer)
    {
        rect.Draw(commandBuffer, {particle});
    });
    particle.Submit();
    device->Queue().waitIdle();

    // Now scan and copy them
    particle.Scan();
    device->Queue().waitIdle();

    std::vector<Particle> outParticlesData(size.x*size.y*8);
    particles.CopyTo(outParticlesData);

    Buffer localCount(*device, vk::BufferUsageFlagBits::eStorageBuffer, true, size.x*size.y*sizeof(int));
    ExecuteCommand(*device, [&](vk::CommandBuffer commandBuffer)
    {
        localCount.CopyFrom(commandBuffer, particle.mCount);
    });

    PrintBuffer<int>(size, localCount);

    // We don't know the order of particles in the same grid position
    ASSERT_TRUE(outParticlesData[0].Position == particlesData[0].Position ||
                outParticlesData[0].Position == particlesData[2].Position);
    ASSERT_TRUE(outParticlesData[1].Position == particlesData[0].Position ||
                outParticlesData[1].Position == particlesData[2].Position);
}

TEST(ParticleTests, ParticleSpawn)
{

}
