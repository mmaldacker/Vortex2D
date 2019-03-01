//
//  Extrapolation.h
//  Vortex2D
//

#ifndef Vortex2d_Extrapolation_h
#define Vortex2d_Extrapolation_h

#include <Vortex2D/Engine/LevelSet.h>
#include <Vortex2D/Engine/Velocity.h>
#include <Vortex2D/Renderer/CommandBuffer.h>
#include <Vortex2D/Renderer/Work.h>

namespace Vortex2D
{
namespace Fluid
{
/**
 * @brief Class to extrapolate values into the neumann and/or dirichlet
 * boundaries
 */
class Extrapolation
{
public:
  VORTEX2D_API Extrapolation(const Renderer::Device& device,
                             const glm::ivec2& size,
                             Renderer::GenericBuffer& valid,
                             Velocity& velocity,
                             int iterations = 10);

  /**
   * @brief Will extrapolate values from buffer into the dirichlet and neumann
   * boundaries
   */
  VORTEX2D_API void Extrapolate();

  /**
   * @brief Binds a solid level set to use later and constrain the velocity
   * against
   * @param solidPhi solid level set
   */
  VORTEX2D_API void ConstrainBind(Renderer::Texture& solidPhi);

  /**
   * @brief Constrain the velocity, i.e. ensure that the velocity normal to the
   * solid level set is 0.
   */
  VORTEX2D_API void ConstrainVelocity();

private:
  Renderer::Buffer<glm::ivec2> mValid;
  Velocity& mVelocity;

  Renderer::Work mExtrapolateVelocity;
  Renderer::Work::Bound mExtrapolateVelocityBound, mExtrapolateVelocityBackBound;
  Renderer::Work mConstrainVelocity;
  Renderer::Work::Bound mConstrainVelocityBound;

  Renderer::CommandBuffer mExtrapolateCmd;
  Renderer::CommandBuffer mConstrainCmd;
};

}  // namespace Fluid
}  // namespace Vortex2D

#endif
