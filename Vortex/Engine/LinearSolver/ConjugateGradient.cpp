//
//  ConjugateGradient.cpp
//  Vortex
//

#include "ConjugateGradient.h"

#include <Vortex/Engine/Rigidbody.h>

#include "vortex_generated_spirv.h"

namespace Vortex
{
namespace Fluid
{
ConjugateGradient::ConjugateGradient(Renderer::Device& device,
                                     const glm::ivec2& size,
                                     Preconditioner& preconditioner)
    : mDevice(device)
    , mPreconditioner(preconditioner)
    , r(device, size.x * size.y)
    , s(device, size.x * size.y)
    , z(device, size.x * size.y)
    , inner(device, size.x * size.y)
    , alpha(device, 1)
    , beta(device, 1)
    , rho(device, 1)
    , rho_new(device, 1)
    , sigma(device, 1)
    , error(device)
    , localError(device, 1, Renderer::MemoryUsage::GpuToCpu)
    , matrixMultiply(device, Renderer::ComputeSize{size}, SPIRV::MultiplyMatrix_comp)
    , scalarDivision(device, Renderer::ComputeSize{glm::ivec2(1)}, SPIRV::Divide_comp)
    , scalarMultiply(device, Renderer::ComputeSize{size}, SPIRV::Multiply_comp)
    , multiplyAdd(device, Renderer::ComputeSize{size}, SPIRV::MultiplyAdd_comp)
    , multiplySub(device, Renderer::ComputeSize{size}, SPIRV::MultiplySub_comp)
    , reduceSum(device, size.x * size.y)
    , reduceMax(device, size.x * size.y)
    , reduceMaxBound(reduceMax.Bind(r, error))
    , reduceSumRhoBound(reduceSum.Bind(inner, rho))
    , reduceSumSigmaBound(reduceSum.Bind(inner, sigma))
    , reduceSumRhoNewBound(reduceSum.Bind(inner, rho_new))
    , multiplySBound(scalarMultiply.Bind({z, s, inner}))
    , multiplyZBound(scalarMultiply.Bind({z, r, inner}))
    , divideRhoBound(scalarDivision.Bind({rho, sigma, alpha}))
    , divideRhoNewBound(scalarDivision.Bind({rho_new, rho, beta}))
    , multiplySubRBound(multiplySub.Bind({r, z, alpha, r}))
    , multiplyAddZBound(multiplyAdd.Bind({z, s, beta, s}))
    , mSolveInit(device, false)
    , mSolve(device, false)
    , mErrorRead(device)
{
  mErrorRead.Record([&](Renderer::CommandEncoder& command)
                    { localError.CopyFrom(command, error); });
}

ConjugateGradient::~ConjugateGradient() {}

void ConjugateGradient::Bind(Renderer::GenericBuffer& d,
                             Renderer::GenericBuffer& l,
                             Renderer::GenericBuffer& b,
                             Renderer::GenericBuffer& pressure)
{
  mPreconditioner.Bind(d, l, r, z);

  matrixMultiplyBound = matrixMultiply.Bind({d, l, s, z});
  multiplyAddPBound = multiplyAdd.Bind({pressure, s, alpha, pressure});

  mSolveInit.Record(
      [&](Renderer::CommandEncoder& command)
      {
        command.DebugMarkerBegin("PCG Init", {0.63f, 0.04f, 0.66f, 1.0f});

        // r = b
        r.CopyFrom(command, b);

        // calculate error
        reduceMaxBound.Record(command);

        // p = 0
        pressure.Clear(command);

        // z = M^-1 r
        z.Clear(command);
        mPreconditioner.Record(command);
        z.Barrier(command, Renderer::Access::Write, Renderer::Access::Read);

        // s = z
        s.CopyFrom(command, z);

        // rho = zTr
        multiplyZBound.Record(command);
        inner.Barrier(command, Renderer::Access::Write, Renderer::Access::Read);
        reduceSumRhoBound.Record(command);
        z.Clear(command);

        command.DebugMarkerEnd();
      });

  mSolve.Record(
      [&](Renderer::CommandEncoder& command)
      {
        command.DebugMarkerBegin("PCG Step", {0.51f, 0.90f, 0.72f, 1.0f});

        // z = As
        matrixMultiplyBound.Record(command);
        z.Barrier(command, Renderer::Access::Write, Renderer::Access::Read);

        // sigma = zTs
        multiplySBound.Record(command);
        inner.Barrier(command, Renderer::Access::Write, Renderer::Access::Read);
        reduceSumSigmaBound.Record(command);

        // alpha = rho / sigma
        divideRhoBound.Record(command);
        alpha.Barrier(command, Renderer::Access::Write, Renderer::Access::Read);

        // p = p + alpha * s
        multiplyAddPBound.Record(command);
        pressure.Barrier(command, Renderer::Access::Write, Renderer::Access::Read);

        // r = r - alpha * z
        multiplySubRBound.Record(command);
        r.Barrier(command, Renderer::Access::Write, Renderer::Access::Read);

        // calculate max error
        reduceMaxBound.Record(command);

        // z = M^-1 r
        z.Clear(command);
        mPreconditioner.Record(command);
        z.Barrier(command, Renderer::Access::Write, Renderer::Access::Read);

        // rho_new = zTr
        multiplyZBound.Record(command);
        inner.Barrier(command, Renderer::Access::Write, Renderer::Access::Read);
        reduceSumRhoNewBound.Record(command);

        // beta = rho_new / rho
        divideRhoNewBound.Record(command);
        beta.Barrier(command, Renderer::Access::Write, Renderer::Access::Read);

        // s = z + beta * s
        multiplyAddZBound.Record(command);
        s.Barrier(command, Renderer::Access::Write, Renderer::Access::Read);
        z.Clear(command);

        // rho = rho_new
        rho.CopyFrom(command, rho_new);

        command.DebugMarkerEnd();
      });
}

void ConjugateGradient::BindRigidbody(float delta, Renderer::GenericBuffer& d, RigidBody& rigidBody)
{
  rigidBody.BindPressure(delta, d, s, z);
}

void ConjugateGradient::Solve(Parameters& params, const std::vector<RigidBody*>& rigidbodies)
{
  params.Reset();

  mSolveInit.Submit();

  if (params.Type == Parameters::SolverType::Iterative)
  {
    mErrorRead.Submit().Wait();

    Renderer::CopyTo(localError, params.OutError);
    if (params.OutError <= params.ErrorTolerance)
    {
      return;
    }

    mErrorRead.Submit();
  }

  auto initialError = params.OutError;
  for (unsigned i = 0; !params.IsFinished(initialError); params.OutIterations = ++i)
  {
    for (auto& rigidbody : rigidbodies)
    {
      rigidbody->Pressure();
    }

    mSolve.Submit();

    if (params.Type == Parameters::SolverType::Iterative)
    {
      mErrorRead.Wait();
      Renderer::CopyTo(localError, params.OutError);
      mErrorRead.Submit();
    }
  }

  if (params.Type == Parameters::SolverType::Iterative)
  {
    mErrorRead.Wait();
  }
}

float ConjugateGradient::GetError()
{
  mErrorRead.Submit().Wait();

  float error;
  Renderer::CopyTo(localError, error);
  return error;
}

}  // namespace Fluid
}  // namespace Vortex
