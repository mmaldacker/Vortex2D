//
//  Reduce.h
//  Vortex2D
//

#ifndef Vortex2D_Reduce_h
#define Vortex2D_Reduce_h

#include <Vortex2D/Renderer/Operator.h>
#include <Vortex2D/Renderer/Data.h>

namespace Vortex2D { namespace Fluid {

/**
 * @brief An Operator class that reduces a Buffer with given operator
 */
class Reduce
{
public:
    /**
     * @brief Runs the reduce operation
     */
    Renderer::OperatorContext operator()(Renderer::Buffer& input);

protected:
    Reduce(glm::vec2 size, const char* fragment);

private:
    std::vector<Renderer::Buffer> s;
    Renderer::Operator reduce;
};

class ReduceSum : public Reduce
{
public:
    ReduceSum(const glm::vec2& size);

    using Reduce::operator();
    Renderer::OperatorContext operator()(Renderer::Buffer& input1, Renderer::Buffer& input2);

private:
    Renderer::Operator mMultiply;
    Renderer::Buffer mReduce;
};

class ReduceMax : public Reduce
{
public:
    ReduceMax(const glm::vec2& size);
};

}}

#endif
