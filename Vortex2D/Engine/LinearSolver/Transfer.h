//
//  Transfer.h
//  Vortex2D
//

#ifndef Vortex2d_Transfer_h
#define Vortex2d_Transfer_h

#include <Vortex2D/Renderer/Work.h>
#include <Vortex2D/Renderer/CommandBuffer.h>

namespace Vortex2D { namespace Fluid {

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
    VORTEX2D_API Transfer(const Renderer::Device& device);

    /**
     * @brief Prolongate a level set on a finer level set. Setting the 4 cells to the value of the coarser grid.
     * Multiple level sets can be bound and indexed.
     * @param level the index of the bound level set to prolongate
     * @param fineSize size of the finer level set
     * @param fine the finer level set
     * @param fineDiagonal the diagonal of the linear equation matrix at size @ref fineSize
     * @param coarse the coarse level set
     * @param coarseDiagonal the diagonal of the linear equation matrix at size half of @ref fineSize
     */
    VORTEX2D_API void ProlongateBind(std::size_t level,
                                     const glm::ivec2& fineSize,
                                     Renderer::GenericBuffer& fine,
                                     Renderer::GenericBuffer& fineDiagonal,
                                     Renderer::GenericBuffer& coarse,
                                     Renderer::GenericBuffer& coarseDiagonal);

    /**
     * @brief Restricing the level set on a coarser level set. Averages 4 cells into one.
     * Multiple level sets can be bound and indexed.
     * @param level the index of the bound level set to prolongate
     * @param fineSize size of the finer level set
     * @param fine the finer level set
     * @param fineDiagonal the diagonal of the linear equation matrix at size @ref fineSize
     * @param coarse the coarse level set
     * @param coarseDiagonal the diagonal of the linear equation matrix at size half of @ref fineSize
     */
    VORTEX2D_API void RestrictBind(std::size_t level,
                                   const glm::ivec2& fineSize,
                                   Renderer::GenericBuffer& fine,
                                   Renderer::GenericBuffer& fineDiagonal,
                                   Renderer::GenericBuffer& coarse,
                                   Renderer::GenericBuffer& coarseDiagonal);

    /**
     * @brief Prolongate the level set, using the bound level sets at the specified index.
     * @param commandBuffer command buffer to record into.
     * @param level index of bound level sets.
     */
    VORTEX2D_API void Prolongate(vk::CommandBuffer commandBuffer, std::size_t level);

    /**
     * @brief Restrict the level set, using the bound level sets at the specified index.
     * @param commandBuffer command buffer to record into.
     * @param level index of bound level sets.
     */
    VORTEX2D_API void Restrict(vk::CommandBuffer commandBuffer, std::size_t level);

private:
    const Renderer::Device& mDevice;
    Renderer::Work mProlongateWork;
    std::vector<Renderer::Work::Bound> mProlongateBound;
    std::vector<Renderer::GenericBuffer*> mProlongateBuffer;

    Renderer::Work mRestrictWork;
    std::vector<Renderer::Work::Bound> mRestrictBound;
    std::vector<Renderer::GenericBuffer*> mRestrictBuffer;
};

}}

#endif
