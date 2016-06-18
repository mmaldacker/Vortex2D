//
//  Operator.h
//  Vortex2D
//

#ifndef Vortex2D_Operator_h
#define Vortex2D_Operator_h

#include "Buffer.h"
#include "Shader.h"
#include <utility>
#include <cassert>

namespace Vortex2D { namespace Fluid {

/**
 * @brief Helper class returned from the call operator of Operator.
 */
struct OperatorContext
{
    OperatorContext(Renderer::Program & p) : Program(p) {}
    Renderer::Program & Program;
};

#define REQUIRES(...) typename std::enable_if<(__VA_ARGS__), int>::type = 0

/**
 * @brief This is a helper class to write succint code when running a shader
 * with multiple Texture inputs on a Buffer
 */
class Operator
{
public:
    /**
     * @brief Constructor with the Vertex and Fragment shader
     * @param vertex the source of the vertex shader (not a filename)
     * @param fragment the source of the fragment shader (not a filename)
     */
    Operator(const char * vertex, const char * fragment) : mProgram(vertex, fragment)
    {}

    /**
     * @brief Use the Program behind the Operator, allows us to set uniforms
     * @return
     */
    Renderer::Program & Use()
    {
        return mProgram.Use();
    }

    /**
     * @brief An overloaded function call operator which takes as argument a list of Buffer objects.
     * The Buffer objects will have their Texture bind in order and a helper class is returned to the Buffer.
     * This Buffer object will then in turn run the Program of this Operator object on its front RenderTexture.
     * @code
     * Buffer input1(size, 1), input2(size, 1), output(size), 1;
     * Operator op(VertexSrc, FragmentSrc);
     * // this will run shader VertexSrc/FragmentSrc with texture input1 & input2
     * // and render on output
     * output = op(input1, intpu2);
     * @endcode
     */
    template<typename... Args>
    OperatorContext operator()(Args && ... args)
    {
        BindHelper(0, std::forward<Args>(args)...);
        return {mProgram};
    }

private:
    template<typename T, typename ... Args, REQUIRES(std::is_same<T, Buffer&>())>
    void BindHelper(int unit, T && input, Args && ... args)
    {
        Front(input).Bind(unit);
        BindHelper(unit+1, std::forward<Args>(args)...);
    }

    template<typename T, typename ... Args, REQUIRES(std::is_same<T, Back>())>
    void BindHelper(int unit, T && input, Args && ... args)
    {
        input.Bind(unit);
        BindHelper(unit+1, std::forward<Args>(args)...);
    }

    void BindHelper(int unit)
    {
    }

    Renderer::Program mProgram;
};

}}

#endif
