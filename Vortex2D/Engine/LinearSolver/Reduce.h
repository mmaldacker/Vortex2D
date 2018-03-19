//
//  Reduce.h
//  Vortex2D
//

#ifndef Vortex2D_Reduce_h
#define Vortex2D_Reduce_h

#include <Vortex2D/Renderer/Work.h>
#include <Vortex2D/Renderer/CommandBuffer.h>

namespace Vortex2D { namespace Fluid {

/**
 * @brief Parallel reduction of a buffer into one value. The operator and type of data is specified by inheriting the class.
 */
class Reduce
{
public:
    virtual ~Reduce() {}

    /**
     * @brief Bound input and output buffer for a reduce operation.
     */
    class Bound
    {
    public:
        Bound() = default;

        /**
         * @brief Record the reduce operation.
         * @param commandBuffer the command buffer to record into.
         */
        void Record(vk::CommandBuffer commandBuffer);

        friend class Reduce;
    private:
        Bound(int size,
              const std::vector<Renderer::CommandBuffer::CommandFn>& bufferBarriers,
              std::vector<Renderer::Work::Bound>&& bounds);

        int mSize;
        std::vector<Renderer::CommandBuffer::CommandFn> mBufferBarriers;
        std::vector<Renderer::Work::Bound> mBounds;
    };

    /**
     * @brief Bind the reduce operation.
     * @param input input buffer
     * @param output output buffer
     * @return a bound object that can be recorded in a command buffer.
     */
    Reduce::Bound Bind(Renderer::GenericBuffer& input, Renderer::GenericBuffer& output);

protected:
    Reduce(const Renderer::Device& device,
           const Renderer::SpirvBinary& spirv,
           const glm::ivec2& size,
           std::size_t typeSize);

private:
    int mSize;
    Renderer::Work mReduce;
    std::vector<Renderer::GenericBuffer> mBuffers;
};

/**
 * @brief Reduce operation on float with addition
 */
class ReduceSum : public Reduce
{
public:
    /**
     * @brief Initialize reduce with device and 2d size
     * @param device
     * @param size
     */
    ReduceSum(const Renderer::Device& device,
              const glm::ivec2& size);
};

/**
 * @brief Reduce operation on a struct with a 2d vector and 1 float (i.e. 3 floats) with addition.
 */
class ReduceJ : public Reduce
{
public:
    /**
     * @brief Initialize reduce with device and 2d size
     * @param device
     * @param size
     */
    ReduceJ(const Renderer::Device& device,
            const glm::ivec2& size);
};

/**
 * @brief Reduce operation on float with max of absolute.
 */
class ReduceMax : public Reduce
{
public:
    /**
     * @brief Initialize reduce with device and 2d size
     * @param device
     * @param size
     */
    ReduceMax(const Renderer::Device& device,
              const glm::ivec2& size);
};

}}

#endif
