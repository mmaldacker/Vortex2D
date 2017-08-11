#include <benchmark/benchmark.h>
#include <Vortex2D/Renderer/Instance.h>
#include <Vortex2D/Renderer/Device.h>
#include <Vortex2D/Renderer/Timer.h>
#include <Vortex2D/Engine/LinearSolver/GaussSeidel.h>

using namespace Vortex2D::Renderer;
using namespace Vortex2D::Fluid;

Vortex2D::Renderer::Device* device;

static void SimpleSOR(benchmark::State& state)
{
    glm::vec2 size(500);

    // TODO fill in data

    Buffer matrix(*device, vk::BufferUsageFlagBits::eStorageBuffer, false, size.x*size.y*sizeof(LinearSolver::Data));
    Buffer div(*device, vk::BufferUsageFlagBits::eStorageBuffer, false, size.x*size.y*sizeof(float));
    Buffer pressure(*device, vk::BufferUsageFlagBits::eStorageBuffer, false, size.x*size.y*sizeof(float));

    Texture liquidPhi(*device, size.x, size.y, vk::Format::eR32Sfloat, false);
    Texture solidPhi(*device, size.x, size.y, vk::Format::eR32Sfloat, false);
    Work matrixBuild(*device, size, "../Vortex2D/BuildMatrix.comp.spv", {vk::DescriptorType::eStorageBuffer,
                                                                         vk::DescriptorType::eStorageImage,
                                                                         vk::DescriptorType::eStorageImage}, 4);

    LinearSolver::Parameters params(300);
    GaussSeidel solver(*device, size);

    Timer timer(*device);

    solver.Init(matrix, div, pressure, matrixBuild, solidPhi, liquidPhi);

    while (state.KeepRunning())
    {
        timer.Start();
        solver.Solve(params);
        timer.Stop();

        state.SetIterationTime(timer.GetElapsedNs());
    }
}

BENCHMARK(SimpleSOR);

int main(int argc, char** argv)
{
    std::vector<const char*> extensions;
    Vortex2D::Renderer::Instance instance_;

    instance_.Create("Tests", extensions, false);
    Vortex2D::Renderer::Device device_(instance_.GetPhysicalDevice());
    device = &device_;

    benchmark::Initialize(&argc, argv);
    benchmark::RunSpecifiedBenchmarks();
}
