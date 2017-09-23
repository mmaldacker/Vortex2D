//
//  ParticleTests.cpp
//  Vortex2D
//

#include "Verify.h"

#include <random>
#include <glm/gtx/io.hpp>

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
    EXPECT_EQ(params.count, total);
    EXPECT_EQ(params.workSize.x, std::ceil((float)total / 256));
    EXPECT_EQ(params.workSize.y, 1);
    EXPECT_EQ(params.workSize.z, 1);
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
    EXPECT_EQ(params.count, total);
    EXPECT_EQ(params.workSize.x, std::ceil((float)total / 256));
    EXPECT_EQ(params.workSize.y, 1);
    EXPECT_EQ(params.workSize.z, 1);
}

TEST(ParticleTests, ParticleCounting)
{
    glm::ivec2 size(20, 20);

    std::vector<Particle> particlesData(size.x*size.y*8);
    particlesData[0].Position = glm::vec2(3.4f, 2.3f);
    particlesData[1].Position = glm::vec2(3.5f, 2.4f);
    particlesData[2].Position = glm::vec2(5.4f, 6.7f);
    int numParticles = 3;

    Buffer particles(*device, vk::BufferUsageFlagBits::eStorageBuffer, true, 8*size.x*size.y*sizeof(Particle));
    particles.CopyFrom(particlesData);

    ParticleCount particleCount(*device, size, particles, {numParticles});

    particleCount.Count();
    device->Handle().waitIdle();

    Texture localCount(*device, size.x, size.y, vk::Format::eR32Sint, true);
    ExecuteCommand(*device, [&](vk::CommandBuffer commandBuffer)
    {
        localCount.CopyFrom(commandBuffer, particleCount);
    });

    std::vector<int> outData(size.x*size.y);
    localCount.CopyTo(outData);

    glm::ivec2 pos1(3, 2);
    glm::ivec2 pos2(5, 6);
    EXPECT_EQ(2, outData[pos1.x + pos1.y * size.x]);
    EXPECT_EQ(1, outData[pos2.x + pos2.y * size.x]);
}

TEST(ParticleTests, ParticleDelete)
{
    glm::ivec2 size(20, 20);

    std::vector<Particle> particlesData(size.x*size.y*8);
    particlesData[0].Position = glm::vec2(3.4f, 2.3f);
    particlesData[1].Position = glm::vec2(13.4f, 16.7f);
    particlesData[2].Position = glm::vec2(3.5f, 2.4f);
    int numParticles = 3;

    Buffer particles(*device, vk::BufferUsageFlagBits::eStorageBuffer, true, 8*size.x*size.y*sizeof(Particle));
    particles.CopyFrom(particlesData);

    ParticleCount particleCount(*device, size, particles, {numParticles});

    // Count particles
    particleCount.Count();
    device->Handle().waitIdle();

    // Delete some particles
    IntRectangle rect(*device, {10, 10}, glm::ivec4(0));
    rect.Position = glm::vec2(10.0f, 10.0f);
    rect.Initialize(particleCount);
    rect.Update(particleCount.Orth, {});

    particleCount.Record([&](vk::CommandBuffer commandBuffer)
    {
        rect.Draw(commandBuffer, {particleCount});
    });
    particleCount.Submit();
    device->Queue().waitIdle();

    // Now scan and copy them
    particleCount.Scan();
    device->Queue().waitIdle();

    ASSERT_EQ(2, particleCount.GetCount());

    // Read particles
    std::vector<Particle> outParticlesData(size.x*size.y*8);
    particles.CopyTo(outParticlesData);

    // We don't know the order of particles in the same grid position
    EXPECT_TRUE(outParticlesData[0].Position == particlesData[0].Position ||
                outParticlesData[0].Position == particlesData[2].Position);
    EXPECT_TRUE(outParticlesData[1].Position == particlesData[0].Position ||
                outParticlesData[1].Position == particlesData[2].Position);
}

TEST(ParticleTests, ParticleSpawn)
{
    glm::ivec2 size(20, 20);

    Buffer particles(*device, vk::BufferUsageFlagBits::eStorageBuffer, true, 8*size.x*size.y*sizeof(Particle));
    ParticleCount particleCount(*device, size, particles);

    // Add some particles
    IntRectangle rect(*device, {1, 1}, glm::ivec4(4));
    rect.Position = glm::vec2(10.0f, 10.0f);
    rect.Initialize(particleCount);
    rect.Update(particleCount.Orth, {});

    particleCount.Record([&](vk::CommandBuffer commandBuffer)
    {
        rect.Draw(commandBuffer, {particleCount});
    });
    particleCount.Submit();
    device->Queue().waitIdle();

    // Now scan and spawn them
    particleCount.Scan();
    device->Queue().waitIdle();

    ASSERT_EQ(4, particleCount.GetCount());

    // Read particles
    std::vector<Particle> outParticlesData(size.x*size.y*8);
    particles.CopyTo(outParticlesData);

    glm::ivec2 particlePos(10, 10);
    EXPECT_EQ(glm::ivec2(outParticlesData[0].Position), particlePos);
    EXPECT_EQ(glm::ivec2(outParticlesData[1].Position), particlePos);
    EXPECT_EQ(glm::ivec2(outParticlesData[2].Position), particlePos);
    EXPECT_EQ(glm::ivec2(outParticlesData[3].Position), particlePos);

    std::vector<glm::vec2> outParticles = {outParticlesData[0].Position,
                                          outParticlesData[1].Position,
                                          outParticlesData[2].Position,
                                          outParticlesData[3].Position};
    std::sort(outParticles.begin(), outParticles.end(),
              [](const auto& left, const auto&right) { return std::tie(left.x, left.y) < std::tie(right.x, right.y); });
    auto it = std::adjacent_find(outParticles.begin(), outParticles.end());
    ASSERT_EQ(it, outParticles.end());
}

TEST(ParticleTests, ParticleAddDelete)
{

    glm::ivec2 size(20, 20);

    Buffer particles(*device, vk::BufferUsageFlagBits::eStorageBuffer, true, 8*size.x*size.y*sizeof(Particle));
    ParticleCount particleCount(*device, size, particles);

    // Add some particles
    IntRectangle rectAdd(*device, {2, 4}, glm::ivec4(1));
    rectAdd.Position = glm::vec2(10.0f, 10.0f);
    rectAdd.Initialize(particleCount);
    rectAdd.Update(particleCount.Orth, {});

    particleCount.Record([&](vk::CommandBuffer commandBuffer)
    {
        rectAdd.Draw(commandBuffer, {particleCount});
    });
    particleCount.Submit();
    device->Queue().waitIdle();

    // Now scan and spawn them
    particleCount.Scan();
    particleCount.Count();
    device->Queue().waitIdle();

    // Remove some particles
    IntRectangle rectRemove(*device, {1, 4}, glm::ivec4(0));
    rectRemove.Position = glm::vec2(10.0f, 10.0f);
    rectRemove.Initialize(particleCount);
    rectRemove.Update(particleCount.Orth, {});

    particleCount.Record([&](vk::CommandBuffer commandBuffer)
    {
        rectRemove.Draw(commandBuffer, {particleCount});
    });
    particleCount.Submit();
    device->Queue().waitIdle();

    // Scan again
    particleCount.Scan();
    device->Queue().waitIdle();

    // Read particles
    std::vector<Particle> outParticlesData(size.x*size.y*8);
    particles.CopyTo(outParticlesData);

    ASSERT_EQ(4, particleCount.GetCount());

    EXPECT_EQ(glm::ivec2(outParticlesData[0].Position), glm::ivec2(11, 10));
    EXPECT_EQ(glm::ivec2(outParticlesData[1].Position), glm::ivec2(11, 11));
    EXPECT_EQ(glm::ivec2(outParticlesData[2].Position), glm::ivec2(11, 12));
    EXPECT_EQ(glm::ivec2(outParticlesData[3].Position), glm::ivec2(11, 13));
}