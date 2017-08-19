#include <benchmark/benchmark.h>

#include <Vortex2D/Renderer/Instance.h>
#include <Vortex2D/Renderer/Device.h>
#include <Vortex2D/Renderer/Timer.h>

#include <Vortex2D/Engine/LinearSolver/GaussSeidel.h>
#include <Vortex2D/Engine/LinearSolver/ConjugateGradient.h>
#include <Vortex2D/Engine/LinearSolver/Diagonal.h>
#include <Vortex2D/Engine/LinearSolver/IncompletePoisson.h>

#include <Tests/Engine/VariationalHelpers.h>

using namespace Vortex2D::Renderer;
using namespace Vortex2D::Fluid;

Vortex2D::Renderer::Device* device;

const glm::ivec2 size = glm::ivec2(500);

FluidSim sim;

void BuildDeviceLocalLinearEquations(const glm::ivec2& size, Vortex2D::Renderer::Buffer& diagonal, Vortex2D::Renderer::Buffer& lower, Vortex2D::Renderer::Buffer& div, FluidSim& sim)
{
    Buffer hostDiagonal(*device, vk::BufferUsageFlagBits::eStorageBuffer, true, size.x*size.y*sizeof(float));
    Buffer hostLower(*device, vk::BufferUsageFlagBits::eStorageBuffer, true, size.x*size.y*sizeof(glm::vec2));
    Buffer hostDiv(*device, vk::BufferUsageFlagBits::eStorageBuffer, true, size.x*size.y*sizeof(float));

    BuildLinearEquation(size, hostDiagonal, hostLower, hostDiv, sim);

    ExecuteCommand(*device, [&](vk::CommandBuffer commandBuffer)
    {
        diagonal.CopyFrom(commandBuffer, hostDiagonal);
        lower.CopyFrom(commandBuffer, hostLower);
        div.CopyFrom(commandBuffer, hostDiv);
    });
}

static void ICCG(benchmark::State& state)
{
    while (state.KeepRunning())
    {
        sim.solve_pressure(0.01f);
    }
}

static void SOR(benchmark::State& state)
{
    Buffer diagonal(*device, vk::BufferUsageFlagBits::eStorageBuffer, false, size.x*size.y*sizeof(float));
    Buffer lower(*device, vk::BufferUsageFlagBits::eStorageBuffer, false, size.x*size.y*sizeof(glm::vec2));
    Buffer div(*device, vk::BufferUsageFlagBits::eStorageBuffer, false, size.x*size.y*sizeof(float));
    Buffer pressure(*device, vk::BufferUsageFlagBits::eStorageBuffer, false, size.x*size.y*sizeof(float));

    BuildDeviceLocalLinearEquations(size, diagonal, lower, div, sim);

    LinearSolver::Parameters params(10000, 1e-4f);
    GaussSeidel solver(*device, size);

    Timer timer(*device);

    solver.Init(diagonal, lower, div, pressure);

    while (state.KeepRunning())
    {
        timer.Start();
        solver.Solve(params);
        timer.Stop();

        device->Queue().waitIdle();

        state.SetIterationTime(timer.GetElapsedNs());
    }

    state.counters["SolveIterations"] = params.OutIterations;
}

static void CG(benchmark::State& state)
{
    Buffer diagonal(*device, vk::BufferUsageFlagBits::eStorageBuffer, false, size.x*size.y*sizeof(float));
    Buffer lower(*device, vk::BufferUsageFlagBits::eStorageBuffer, false, size.x*size.y*sizeof(glm::vec2));
    Buffer div(*device, vk::BufferUsageFlagBits::eStorageBuffer, false, size.x*size.y*sizeof(float));
    Buffer pressure(*device, vk::BufferUsageFlagBits::eStorageBuffer, false, size.x*size.y*sizeof(float));

    BuildDeviceLocalLinearEquations(size, diagonal, lower, div, sim);

    Diagonal preconditioner(*device, size);

    LinearSolver::Parameters params(10000, 1e-4f);
    ConjugateGradient solver(*device, size, preconditioner);

    Timer timer(*device);

    solver.Init(diagonal, lower, div, pressure);

    while (state.KeepRunning())
    {
        timer.Start();
        solver.NormalSolve(params);
        timer.Stop();

        device->Queue().waitIdle();

        state.SetIterationTime(timer.GetElapsedNs());
    }

    state.counters["SolveIterations"] = params.OutIterations;
}

static void DiagonalCG(benchmark::State& state)
{
    Buffer diagonal(*device, vk::BufferUsageFlagBits::eStorageBuffer, false, size.x*size.y*sizeof(float));
    Buffer lower(*device, vk::BufferUsageFlagBits::eStorageBuffer, false, size.x*size.y*sizeof(glm::vec2));
    Buffer div(*device, vk::BufferUsageFlagBits::eStorageBuffer, false, size.x*size.y*sizeof(float));
    Buffer pressure(*device, vk::BufferUsageFlagBits::eStorageBuffer, false, size.x*size.y*sizeof(float));

    BuildDeviceLocalLinearEquations(size, diagonal, lower, div, sim);

    Diagonal preconditioner(*device, size);

    LinearSolver::Parameters params(10000, 1e-4f);
    ConjugateGradient solver(*device, size, preconditioner);

    Timer timer(*device);

    solver.Init(diagonal, lower, div, pressure);

    while (state.KeepRunning())
    {
        timer.Start();
        solver.Solve(params);
        timer.Stop();

        device->Queue().waitIdle();

        state.SetIterationTime(timer.GetElapsedNs());
    }

    state.counters["SolveIterations"] = params.OutIterations;
}

static void IncompletePoissonCG(benchmark::State& state)
{
    Buffer diagonal(*device, vk::BufferUsageFlagBits::eStorageBuffer, false, size.x*size.y*sizeof(float));
    Buffer lower(*device, vk::BufferUsageFlagBits::eStorageBuffer, false, size.x*size.y*sizeof(glm::vec2));
    Buffer div(*device, vk::BufferUsageFlagBits::eStorageBuffer, false, size.x*size.y*sizeof(float));
    Buffer pressure(*device, vk::BufferUsageFlagBits::eStorageBuffer, false, size.x*size.y*sizeof(float));

    BuildDeviceLocalLinearEquations(size, diagonal, lower, div, sim);

    IncompletePoisson preconditioner(*device, size);

    LinearSolver::Parameters params(10000, 1e-4f);
    ConjugateGradient solver(*device, size, preconditioner);

    Timer timer(*device);

    solver.Init(diagonal, lower, div, pressure);

    while (state.KeepRunning())
    {
        timer.Start();
        solver.Solve(params);
        timer.Stop();

        device->Queue().waitIdle();

        state.SetIterationTime(timer.GetElapsedNs());
    }

    auto statistics = solver.GetStatistics();
    for (auto statistic: statistics)
    {
        std::cout << statistic.first << ": " << (float)statistic.second / 1000 << std::endl;
    }

    state.counters["SolveIterations"] = params.OutIterations;
}

static void GaussSeidelCG(benchmark::State& state)
{
    Buffer diagonal(*device, vk::BufferUsageFlagBits::eStorageBuffer, false, size.x*size.y*sizeof(float));
    Buffer lower(*device, vk::BufferUsageFlagBits::eStorageBuffer, false, size.x*size.y*sizeof(glm::vec2));
    Buffer div(*device, vk::BufferUsageFlagBits::eStorageBuffer, false, size.x*size.y*sizeof(float));
    Buffer pressure(*device, vk::BufferUsageFlagBits::eStorageBuffer, false, size.x*size.y*sizeof(float));

    BuildDeviceLocalLinearEquations(size, diagonal, lower, div, sim);

    GaussSeidel preconditioner(*device, size);
    preconditioner.SetW(1.5f);
    preconditioner.SetPreconditionerIterations(state.range(0));

    LinearSolver::Parameters params(10000, 1e-4f);
    ConjugateGradient solver(*device, size, preconditioner);

    Timer timer(*device);

    solver.Init(diagonal, lower, div, pressure);

    while (state.KeepRunning())
    {
        timer.Start();
        solver.Solve(params);
        timer.Stop();

        device->Queue().waitIdle();

        state.SetIterationTime(timer.GetElapsedNs());
    }

    auto statistics = solver.GetStatistics();
    for (auto statistic: statistics)
    {
        std::cout << statistic.first << ": " << (float)statistic.second / 1000 << std::endl;
    }

    state.counters["SolveIterations"] = params.OutIterations;
}

BENCHMARK(ICCG)->Unit(benchmark::kMillisecond);
BENCHMARK(SOR)->Unit(benchmark::kMillisecond);
BENCHMARK(CG)->Unit(benchmark::kMillisecond);
BENCHMARK(DiagonalCG)->Unit(benchmark::kMillisecond);
BENCHMARK(IncompletePoissonCG)->Unit(benchmark::kMillisecond);
BENCHMARK(GaussSeidelCG)->RangeMultiplier(2)->Range(1, 64)->Unit(benchmark::kMillisecond);

int main(int argc, char** argv)
{
    // init vulkan
    std::vector<const char*> extensions;
    Vortex2D::Renderer::Instance instance_;

    instance_.Create("Tests", extensions, false);
    Vortex2D::Renderer::Device device_(instance_.GetPhysicalDevice());
    device = &device_;

    // create matrix
    sim.initialize(1.0f, size.x, size.y);
    sim.set_boundary(complex_boundary_phi);

    AddParticles(size, sim, complex_boundary_phi);

    sim.add_force(0.01f);
    sim.compute_phi();
    sim.extrapolate_phi();
    sim.compute_weights();
    sim.compute_linear_equations(0.01f);

    benchmark::Initialize(&argc, argv);
    benchmark::RunSpecifiedBenchmarks();
}
