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
#include "Reader.h"
#include <utility>

namespace Fluid
{

template<typename T>
struct Expr : T
{
    template<typename... Args>
    Expr(Args&& ... args) : T(std::forward<Args>(args)...) {}
};

struct Empty{ void operator()(const glm::mat4 &) {} };
using EmptyExpr = Expr<Empty>;

template<typename T, typename U>
struct And
{
    And(Expr<T> l, Expr<U> r) : left(l), right(r) {}

    void operator()(const glm::mat4 & ortho){ left(ortho); right(ortho); }

    Expr<T> left; Expr<U> right;
};
template<typename T, typename U>
using AndExpr = Expr<And<T,U>>;

struct Render
{
    Render(Renderer::Program & p, Renderer::Quad & q) : program(p), quad(q) {}

    void operator()(const glm::mat4 & ortho)
    {
        program.Use().SetMVP(ortho);
        quad.Render();
    }

    Renderer::Program & program;
    Renderer::Quad & quad;
};
using RenderExpr = Expr<Render>;

template<typename T>
struct Context
{
    Context(Renderer::Program & p, Expr<T> bind) : program(p), bind(bind) {}

    AndExpr<Expr<T>, RenderExpr> render(Renderer::Quad & quad)
    {
        return {bind, RenderExpr(program, quad)};
    }

    Renderer::Program & program;
    Expr<T> bind;
};

class Buffer
{
public:
    Buffer(const glm::vec2 & size, unsigned components)
    : mTexture(size.x, size.y,
               components == 1 ? Renderer::Texture::PixelFormat::RF :
               components == 2 ? Renderer::Texture::PixelFormat::RGF : Renderer::Texture::PixelFormat::RGBAF)
    , mQuad(size)
    {
        Orth = mTexture.Orth;
    }

    template<typename T>
    Buffer & operator=(Context<Expr<T>> expr)
    {
        mTexture.begin();
        expr.render(mQuad)(mTexture.Orth);
        mTexture.end();
        return *this;
    }

    Renderer::Reader get()
    {
        return {mTexture};
    }

    const glm::vec2 & size() const
    {
        return mQuad.Size();
    }

    // FIXME hide those or remove those?
    void bind(int n = 0) { mTexture.Bind(n); }
    void begin() { mTexture.begin(); }
    void begin(const glm::vec4 & c) { mTexture.begin(c); }
    void end() { mTexture.end(); }
    void clear() { mTexture.Clear(); }
    
    glm::mat4 Orth;
    
private:
    Renderer::RenderTexture mTexture;
    Renderer::Quad mQuad;
};

struct Bind
{
    Bind(Buffer & b, int n = 0) : n(n), buffer(b) {}

    void operator()(const glm::mat4 &) { buffer.bind(n); }

    int n;
    Buffer & buffer;
};
using BindExpr = Expr<Bind>;

template<typename...P> struct bind_type;
template<> struct bind_type<>{ typedef EmptyExpr type; };
template<typename T, typename...P> struct bind_type<T, P...>
{ typedef AndExpr<typename bind_type<P...>::type, BindExpr> type; };

class Operator
{
public:
    Operator(const std::string & vertex, const std::string & fragment) : mProgram(vertex, fragment)
    {}

    Renderer::Program & Use()
    {
        return mProgram.Use();
    }

    template<typename... Args>
    Context<typename bind_type<Args...>::type> operator()(Args& ... args)
    {
        return {mProgram, bind(0, args...)};
    }

private:
    template<typename T, typename ... Args>
    typename bind_type<T, Args...>::type bind(int unit, T & input, Args & ... args)
    {
        return {bind(unit+1, args...), BindExpr(input, unit)};
    }

    EmptyExpr bind(int unit)
    {
        return {};
    }

    Renderer::Program mProgram;
};

}

#endif
