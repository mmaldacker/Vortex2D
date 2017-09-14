//
//  ParticleTests.cpp
//  Vortex2D
//

#include "Verify.h"

#include <random>

#include <Vortex2D/Engine/PrefixScan.h>

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

    std::vector<int> inputData = GenerateInput(size.x*size.y);
    input.CopyFrom(inputData);

    auto bound = prefixScan.Bind(input, output);

    ExecuteCommand(*device, [&](vk::CommandBuffer commandBuffer)
    {
        bound.Record(commandBuffer);
    });

    auto outputData = CalculatePrefixScan(inputData);
    CheckBuffer(outputData, output);
}

TEST(ParticleTests, PrefixScanBig)
{
    glm::ivec2 size(100, 100);

    PrefixScan prefixScan(*device, size);

    Buffer input(*device, vk::BufferUsageFlagBits::eStorageBuffer, true, size.x*size.y*sizeof(int));
    Buffer output(*device, vk::BufferUsageFlagBits::eStorageBuffer, true, size.x*size.y*sizeof(int));

    std::vector<int> inputData = GenerateInput(size.x*size.y);
    input.CopyFrom(inputData);

    auto bound = prefixScan.Bind(input, output);

    ExecuteCommand(*device, [&](vk::CommandBuffer commandBuffer)
    {
        bound.Record(commandBuffer);
    });

    auto outputData = CalculatePrefixScan(inputData);
    CheckBuffer(outputData, output);
}
