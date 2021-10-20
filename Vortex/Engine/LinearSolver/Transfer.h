//
//  Transfer.h
//  Vortex
//

#pragma once

#include <Vortex/Renderer/CommandBuffer.h>
#include <Vortex/Renderer/Work.h>

namespace Vortex
{
namespace Fluid
{
/**
 * @brief Prolongates or restrict a level set on a finer or coarser level set.
 */
class Transfer
{
public:
  /**
   * @brief Initialize prolongate and restrict compute pipelines
   * @param device
   */
  VORTEX_API Transfer(Renderer::Device& device);

  /**
   * @brief Prolongate a level set on a finer level set. Setting the 4 cells to
   * the value of the coarser grid. Multiple level sets can be bound and
   * indexed.
   * @param level the index of the bound level set to prolongate
   * @param fineSize size of the finer level set
   * @param fine the finer level set
   * @param fineDiagonal the diagonal of the linear equation matrix at size @p
   * fineSize
   * @param coarse the coarse level set
   * @param coarseDiagonal the diagonal of the linear equation matrix at size
   * half of @p fineSize
   */
  VORTEX_API void ProlongateBind(std::size_t level,
                                 const glm::ivec2& fineSize,
                                 Renderer::GenericBuffer& fine,
                                 Renderer::GenericBuffer& fineDiagonal,
                                 Renderer::GenericBuffer& coarse,
                                 Renderer::GenericBuffer& coarseDiagonal);

  /**
   * @brief Restricing the level set on a coarser level set. Averages 4 cells
   * into one. Multiple level sets can be bound and indexed.
   * @param level the index of the bound level set to prolongate
   * @param fineSize size of the finer level set
   * @param fine the finer level set
   * @param fineDiagonal the diagonal of the linear equation matrix at size @p
   * fineSize
   * @param coarse the coarse level set
   * @param coarseDiagonal the diagonal of the linear equation matrix at size
   * half of @p fineSize
   */
  VORTEX_API void RestrictBind(std::size_t level,
                               const glm::ivec2& fineSize,
                               Renderer::GenericBuffer& fine,
                               Renderer::GenericBuffer& fineDiagonal,
                               Renderer::GenericBuffer& coarse,
                               Renderer::GenericBuffer& coarseDiagonal);

  /**
   * @brief Prolongate the level set, using the bound level sets at the
   * specified index.
   * @param commandBuffer command buffer to record into.
   * @param level index of bound level sets.
   */
  VORTEX_API void Prolongate(vk::CommandBuffer commandBuffer, std::size_t level);

  /**
   * @brief Restrict the level set, using the bound level sets at the specified
   * index.
   * @param commandBuffer command buffer to record into.
   * @param level index of bound level sets.
   */
  VORTEX_API void Restrict(vk::CommandBuffer commandBuffer, std::size_t level);

private:
  Renderer::Device& mDevice;
  Renderer::Work mProlongateWork;
  std::vector<Renderer::Work::Bound> mProlongateBound;
  std::vector<Renderer::GenericBuffer*> mProlongateBuffer;

  Renderer::Work mRestrictWork;
  std::vector<Renderer::Work::Bound> mRestrictBound;
  std::vector<Renderer::GenericBuffer*> mRestrictBuffer;
};

}  // namespace Fluid
}  // namespace Vortex
