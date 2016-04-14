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

struct Context
{
    Context(Renderer::Program & p) : program(p) {}

    void render(Renderer::Quad & quad, const glm::mat4 & ortho)
    {
        program.Use().SetMVP(ortho);
        quad.Render();
    }

    Renderer::Program & program;
};

class Buffer
{
public:
    Buffer(const glm::vec2 & size, unsigned components, bool doubled = false, bool depth = false)
        : Quad(size)
    {
        add(size, components, depth);
        if(doubled) add(size, components, depth);

        Orth = mTextures.front().Orth;
    }

    Buffer & operator=(Context context)
    {
        begin();
        context.render(Quad, Orth);
        end();
        return *this;
    }

    Renderer::Reader get()
    {
        return {mTextures.front()};
    }

    const glm::vec2 & size() const
    {
        return Quad.Size();
    }

    void linear()
    {
        for(auto && t : mTextures) t.SetAntiAliasTexParameters();
    }

    void clamp_to_edge()
    {
        for(auto && t : mTextures) t.SetClampToEdgeTexParameters();
    }

    void begin()
    {
        mTextures.front().begin();
    }

    void begin(const glm::vec4 & c)
    {
        mTextures.front().begin(c);
    }

    void end()
    {
        mTextures.front().end();
    }
    
    void clear()
    {
        for(auto && t : mTextures) t.Clear();
    }

    Buffer & swap()
    {
        assert(mTextures.size() == 2);
        std::swap(mTextures.front(), mTextures.back());
        return *this;
    }

    Renderer::Texture & texture()
    {
        return mTextures.front();
    }
    
    glm::mat4 Orth;
    Renderer::Quad Quad;

    friend struct Back;
    friend struct Front;
private:
    void add(const glm::vec2 & size, unsigned components, bool depth)
    {
        mTextures.emplace_back(size.x, size.y,
                               components == 1 ? Renderer::Texture::PixelFormat::RF :
                               components == 2 ? Renderer::Texture::PixelFormat::RGF :
                                                 Renderer::Texture::PixelFormat::RGBAF,
                               depth ? Renderer::RenderTexture::DepthFormat::DEPTH24_STENCIL8 :
                                       Renderer::RenderTexture::DepthFormat::NONE);
        mTextures.back().SetAliasTexParameters();
    }

    // of size 1 or 2
    std::vector<Renderer::RenderTexture> mTextures;
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
        bind_helper(0, std::forward<Args>(args)...);
        return {mProgram};
    }

private:
    template<typename T, typename ... Args, REQUIRES(std::is_same<T, Buffer&>())>
    void bind_helper(int unit, T && input, Args && ... args)
    {
        Front(input).bind(unit);
        bind_helper(unit+1, std::forward<Args>(args)...);
    }

    template<typename T, typename ... Args, REQUIRES(std::is_same<T, Back>())>
    void bind_helper(int unit, T && input, Args && ... args)
    {
        input.bind(unit);
        bind_helper(unit+1, std::forward<Args>(args)...);
    }

    void bind_helper(int unit)
    {
    }

    Renderer::Program mProgram;
};

}

#endif
