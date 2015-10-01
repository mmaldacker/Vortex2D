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

struct Empty{ void operator()() {} };
using EmptyExpr = Expr<Empty>;

template<typename T, typename U>
struct And
{
    And(Expr<T> l, Expr<U> r) : left(l), right(r) {}

    void operator()(){ left(); right(); }

    Expr<T> left; Expr<U> right;
};
template<typename T, typename U>
using AndExpr = Expr<And<T,U>>;

struct Render
{
    Render(Renderer::Program & p, Renderer::Quad & q, const glm::mat4 & o) : program(p), quad(q), ortho(o) {}

    void operator()()
    {
        program.Use().SetMVP(ortho);
        quad.Render();
    }

    Renderer::Program & program;
    Renderer::Quad & quad;
    const glm::mat4 & ortho;
};
using RenderExpr = Expr<Render>;

template<typename T>
struct Context
{
    Context(Renderer::Program & p, Expr<T> bind) : program(p), bind(bind) {}

    AndExpr<Expr<T>, RenderExpr> render(Renderer::Quad & quad, const glm::mat4 & ortho)
    {
        return {bind, RenderExpr(program, quad, ortho)};
    }

    Renderer::Program & program;
    Expr<T> bind;
};

class Buffer
{
public:
    Buffer(const glm::vec2 & size, unsigned components, bool doubled = false, bool depth = false)
    : mQuad(size)
    {
        add(size, components, depth);
        if(doubled) add(size, components, depth);

        Orth = mTextures.front().Orth;
    }

    template<typename T>
    Buffer & operator=(Context<Expr<T>> expr)
    {
        mTextures.front().begin();
        expr.render(mQuad, Orth)();
        mTextures.front().end();
        return *this;
    }

    Renderer::Reader get()
    {
        return {mTextures.front()};
    }

    const glm::vec2 & size() const
    {
        return mQuad.Size();
    }

    void linear()
    {
        for(auto && t : mTextures) t.SetAntiAliasTexParameters();
    }

    // FIXME hide those or remove those?
    void begin() { mTextures.front().begin(); }
    void begin(const glm::vec4 & c) { mTextures.front().begin(c); }
    void end() { mTextures.front().end(); }
    void clear() { for(auto && t : mTextures) t.Clear(); }
    void swap() { assert(mTextures.size() == 2); std::swap(mTextures.front(), mTextures.back()); }
    
    glm::mat4 Orth;

    friend struct Back;
    friend struct Front;
private:
    void add(const glm::vec2 & size, unsigned components, bool depth)
    {
        mTextures.emplace_back(size.x, size.y,
                               components == 1 ? Renderer::Texture::PixelFormat::RF :
                               components == 2 ? Renderer::Texture::PixelFormat::RGF : Renderer::Texture::PixelFormat::RGBAF,
                               depth ? Renderer::RenderTexture::DepthFormat::DEPTH24_STENCIL8 : Renderer::RenderTexture::DepthFormat::NONE);
        mTextures.back().SetAliasTexParameters();
    }

    // of size 1 or 2
    std::vector<Renderer::RenderTexture> mTextures;
    Renderer::Quad mQuad;
};

struct Front
{
    Front(Buffer & b) : buffer(b) {}

    void bind(int n) { buffer.mTextures.front().Bind(n); }

    Buffer & buffer;
};

struct Back
{
    explicit Back(Buffer & b) : buffer(b) {}

    void bind(int n) { buffer.mTextures.back().Bind(n); }

    Buffer & buffer;
};

template<typename T>
struct Bind
{
    Bind(T && b, int n = 0) : n(n), buffer(std::move(b)) {}

    void operator()() { buffer.bind(n); }

    int n;
    T buffer;
};

template<typename T>
using BindExpr = Expr<Bind<T>>;

template<typename...P>
struct bind_type;

template<>
struct bind_type<>
{
    typedef EmptyExpr type;
};

template<typename...P>
struct bind_type<Buffer&, P...>
{
    typedef AndExpr<typename bind_type<P...>::type, BindExpr<Front>> type;
};

template<typename...P>
struct bind_type<Back, P...>
{
    typedef AndExpr<typename bind_type<P...>::type, BindExpr<Back>> type;
};

#define REQUIRES(...) typename std::enable_if<(__VA_ARGS__), int>::type = 0

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
    Context<typename bind_type<Args...>::type> operator()(Args&& ... args)
    {
        return {mProgram, bind(0, std::forward<Args>(args)...)};
    }

private:
    template<typename T, typename ... Args, REQUIRES(std::is_same<T, Buffer&>())>
    typename bind_type<T, Args...>::type bind(int unit, T && input, Args && ... args)
    {
        return {bind(unit+1, std::forward<Args>(args)...), BindExpr<Front>(Front(input), unit)};
    }

    template<typename T, typename ... Args, REQUIRES(std::is_same<T, Back>())>
    typename bind_type<T, Args...>::type bind(int unit, T && input, Args && ... args)
    {
        return {bind(unit+1, std::forward<Args>(args)...), BindExpr<Back>(std::move(input), unit)};
    }

    EmptyExpr bind(int unit)
    {
        return {};
    }

    Renderer::Program mProgram;
};

}

#endif
