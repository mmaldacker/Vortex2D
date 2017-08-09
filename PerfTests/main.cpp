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

    Buffer data(*device, vk::BufferUsageFlagBits::eStorageBuffer, false, size.x*size.y*sizeof(LinearSolver::Data));
    Buffer pressure(*device, vk::BufferUsageFlagBits::eStorageBuffer, false, size.x*size.y*sizeof(float));

    LinearSolver::Parameters params(300);
    GaussSeidel solver(*device, size);

    Timer timer(*device);

    solver.Init(data, pressure);

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
