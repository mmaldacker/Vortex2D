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
#include "Sprite.h"

namespace Fluid
{

struct OperatorContext;

class Buffer : public Renderer::RenderTarget
{
public:
    Buffer(const glm::vec2 & size, unsigned components, bool doubled = false, bool depth = false);

    Buffer & operator=(OperatorContext context);

    Renderer::Reader Get();

    void Linear();
    void ClampToEdge();

    void Clear(const glm::vec4 & colour) override;
    void Render(Renderer::Drawable & object, const glm::mat4 & transform = glm::mat4()) override;
    void ClearStencil();

    Buffer & Swap();

    Renderer::Sprite & Sprite();

    friend struct Back;
    friend struct Front;
private:
    void Add(const glm::vec2 & size, unsigned components, bool depth);
    
    // of size 1 or 2
    std::vector<Renderer::RenderTexture> mTextures;
    Renderer::Sprite mSprite;
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
