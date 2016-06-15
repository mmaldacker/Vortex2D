//
//  Buffer.h
//  Vortex2D
//
//  Created by Maximilian Maldacker on 15/06/2016.
//
//

#ifndef Buffer_h
#define Buffer_h

#include "RenderTarget.h"
#include "Reader.h"

namespace Fluid
{

class Quad
{
public:
    Quad(const glm::vec2 & size);
    Quad(Quad &&);
    ~Quad();

    void Render();

private:
    GLuint mVertexArray;
    GLuint mVertexBuffer;
};

struct Context;

class Buffer : public Renderer::RenderTarget
{
public:
    Buffer(const glm::vec2 & size, unsigned components, bool doubled = false, bool depth = false);

    Buffer & operator=(Context context);

    Renderer::Reader Get();

    void Linear();
    void ClampToEdge();

    void Clear(const glm::vec4 & colour) override;
    void Render(const Renderer::DrawablesVector & objects, const glm::mat4 & transform) override;
    void ClearStencil();

    Buffer & Swap();

    Renderer::Texture & Texture();
    void Render();

    friend struct Back;
    friend struct Front;
private:
    void Add(const glm::vec2 & size, unsigned components, bool depth);
    
    // of size 1 or 2
    std::vector<Renderer::RenderTexture> mTextures;
    Quad mQuad;
};

struct Front
{
    Front(Buffer & b) : buffer(b) {}

    void Bind(int n) { buffer.mTextures.front().Bind(n); }

    Buffer & buffer;
};

struct Back
{
    explicit Back(Buffer & b) : buffer(b) {}

    void Bind(int n) { buffer.mTextures.back().Bind(n); }
    
    Buffer & buffer;
};

}

#endif /* Buffer_h */
