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

void BuildDeviceLocalLinearEquations(const glm::ivec2& size, Vortex2D::Renderer::Buffer& matrix, Vortex2D::Renderer::Buffer& div, FluidSim& sim)
{
    Buffer hostMatrix(*device, vk::BufferUsageFlagBits::eStorageBuffer, true, size.x*size.y*sizeof(LinearSolver::Data));
    Buffer hostDiv(*device, vk::BufferUsageFlagBits::eStorageBuffer, true, size.x*size.y*sizeof(float));

    BuildLinearEquation(size, hostMatrix, hostDiv, sim);

    ExecuteCommand(*device, [&](vk::CommandBuffer commandBuffer)
    {
        matrix.CopyFrom(commandBuffer, hostMatrix);
        div.CopyFrom(commandBuffer, hostDiv);
    });
}

static void SOR(benchmark::State& state)
{
    Buffer matrix(*device, vk::BufferUsageFlagBits::eStorageBuffer, false, size.x*size.y*sizeof(LinearSolver::Data));
    Buffer div(*device, vk::BufferUsageFlagBits::eStorageBuffer, false, size.x*size.y*sizeof(float));
    Buffer pressure(*device, vk::BufferUsageFlagBits::eStorageBuffer, false, size.x*size.y*sizeof(float));

    BuildDeviceLocalLinearEquations(size, matrix, div, sim);

    LinearSolver::Parameters params(10000, 1e-4f);
    GaussSeidel solver(*device, size);

    Timer timer(*device);

    solver.Init(matrix, div, pressure);

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
    Buffer matrix(*device, vk::BufferUsageFlagBits::eStorageBuffer, false, size.x*size.y*sizeof(LinearSolver::Data));
    Buffer div(*device, vk::BufferUsageFlagBits::eStorageBuffer, false, size.x*size.y*sizeof(float));
    Buffer pressure(*device, vk::BufferUsageFlagBits::eStorageBuffer, false, size.x*size.y*sizeof(float));

    BuildDeviceLocalLinearEquations(size, matrix, div, sim);

    Diagonal preconditioner(*device, size);

    LinearSolver::Parameters params(10000, 1e-4f);
    ConjugateGradient solver(*device, size, preconditioner);

    Timer timer(*device);

    solver.Init(matrix, div, pressure);

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
    Buffer matrix(*device, vk::BufferUsageFlagBits::eStorageBuffer, false, size.x*size.y*sizeof(LinearSolver::Data));
    Buffer div(*device, vk::BufferUsageFlagBits::eStorageBuffer, false, size.x*size.y*sizeof(float));
    Buffer pressure(*device, vk::BufferUsageFlagBits::eStorageBuffer, false, size.x*size.y*sizeof(float));

    BuildDeviceLocalLinearEquations(size, matrix, div, sim);

    Diagonal preconditioner(*device, size);

    LinearSolver::Parameters params(10000, 1e-4f);
    ConjugateGradient solver(*device, size, preconditioner);

    Timer timer(*device);

    solver.Init(matrix, div, pressure);

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
    Buffer matrix(*device, vk::BufferUsageFlagBits::eStorageBuffer, false, size.x*size.y*sizeof(LinearSolver::Data));
    Buffer div(*device, vk::BufferUsageFlagBits::eStorageBuffer, false, size.x*size.y*sizeof(float));
    Buffer pressure(*device, vk::BufferUsageFlagBits::eStorageBuffer, false, size.x*size.y*sizeof(float));

    BuildDeviceLocalLinearEquations(size, matrix, div, sim);

    IncompletePoisson preconditioner(*device, size);

    LinearSolver::Parameters params(10000, 1e-4f);
    ConjugateGradient solver(*device, size, preconditioner);

    Timer timer(*device);

    solver.Init(matrix, div, pressure);

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

static void GaussSeidelCG(benchmark::State& state)
{
    Buffer matrix(*device, vk::BufferUsageFlagBits::eStorageBuffer, false, size.x*size.y*sizeof(LinearSolver::Data));
    Buffer div(*device, vk::BufferUsageFlagBits::eStorageBuffer, false, size.x*size.y*sizeof(float));
    Buffer pressure(*device, vk::BufferUsageFlagBits::eStorageBuffer, false, size.x*size.y*sizeof(float));

    BuildDeviceLocalLinearEquations(size, matrix, div, sim);

    GaussSeidel preconditioner(*device, size);
    preconditioner.SetW(1.0f);
    preconditioner.SetPreconditionerIterations(state.range(0));

    LinearSolver::Parameters params(10000, 1e-4f);
    ConjugateGradient solver(*device, size, preconditioner);

    Timer timer(*device);

    solver.Init(matrix, div, pressure);

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
