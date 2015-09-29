//
//  Operator.h
//  Vertex2D
//
//  Created by Maximilian Maldacker on 16/09/2015.
//  Copyright (c) 2015 Maximilian Maldacker. All rights reserved.
//

#ifndef Vertex2D_Operator_h
#define Vertex2D_Operator_h

#include "RenderTexture.h"
#include "Shader.h"
#include "Quad.h"

namespace Fluid
{

class Operator
{
public:
    Operator(const std::string & vertex, const std::string & fragment) : mProgram(vertex, fragment)
    {}

    template<typename Output, typename ... Args>
    void apply(Renderer::Quad & quad, Output & output, Args & ... args)
    {
        output.begin();
        bind(0, args...);
        mProgram.Use().SetMVP(output.Orth);
        quad.Render();
        mProgram.Unuse();
        output.end();
    }

    Renderer::Program & Use()
    {
        return mProgram.Use();
    }

private:
    template<typename T, typename ... Args>
    void bind(int unit, T & input, Args & ... args)
    {
        input.Bind(unit);
        bind(unit+1, args...);
    }

    void bind(int unit)
    {
    }

    Renderer::Program mProgram;
};

}

#endif
