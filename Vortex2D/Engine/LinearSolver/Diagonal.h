//
//  Diagonal.h
//  Vortex2D
//

#ifndef Vortex2D_Diagonal_h
#define Vortex2D_Diagonal_h

#include <Vortex2D/Engine/LinearSolver/Preconditioner.h>
#include <Vortex2D/Renderer/Work.h>
#include <Vortex2D/Renderer/CommandBuffer.h>

namespace Vortex2D { namespace Fluid {


class Diagonal : public Preconditioner
{
public:
    Diagonal(const Renderer::Device& device, const glm::ivec2& size);

    void Init(Renderer::Buffer& A,
              Renderer::Buffer& b,
              Renderer::Buffer& pressure,
              Renderer::Work& buildMatrix,
              Renderer::Texture& solidPhi,
              Renderer::Texture& liquidPhi) override;

    void Record(vk::CommandBuffer ) override;

private:
    Renderer::Work mDiagonal;
    Renderer::Work::Bound mDiagonalBound;
};

}}

#endif
