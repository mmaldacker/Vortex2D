//
//  Operator.h
//  Vertex2D
//
//  Created by Maximilian Maldacker on 16/09/2015.
//  Copyright (c) 2015 Maximilian Maldacker. All rights reserved.
//

#ifndef Vertex2D_Operator_h
#define Vertex2D_Operator_h

#include "Buffer.h"
#include "Shader.h"
#include <utility>
#include <cassert>

namespace Fluid
{

struct Context : Renderer::Drawable
{
    Context(Renderer::Program & p) : Program(p) {}

    void Render(Renderer::RenderTarget & target, const glm::mat4 & transform) override
    {
        Program.Use().SetMVP(target.Orth);
        assert(Surface);
        Surface->Render();
    }

    Renderer::Program & Program;
    Quad * Surface = nullptr;
};

#define REQUIRES(...) typename std::enable_if<(__VA_ARGS__), int>::type = 0

class Operator
{
public:
    Operator(const char * vertex, const char * fragment) : mProgram(vertex, fragment)
    {}

    Renderer::Program & Use()
    {
        return mProgram.Use();
    }

    template<typename... Args>
    Context operator()(Args && ... args)
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

}

#endif
