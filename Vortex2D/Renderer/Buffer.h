//
//  Buffer.h
//  Vortex2D
//

#ifndef Buffer_h
#define Buffer_h

#include "RenderTarget.h"
#include "RenderTexture.h"
#include "Sprite.h"

namespace Vortex2D { namespace Renderer {

struct OperatorContext;

/**
 * @brief Represents the grid used to hold the velocity field, boundaries, density, etc.
 * This holds one or two RenderTexture which allows the use of ping-pong rendering: one RenderTexture
 * is used as input while the other is used as output, when we're done we swap the two. This is used
 * for iterative algorithms such as the LinearSolvers.
 * The equal operator is overloaded and works with the Operator class and makes for succint code when
 * running a shader with multiple inputs.
 */
class Buffer : public Renderer::RenderTarget
{
public:
    /**
     * @brief Constructor that creates the underlying RenderTexture(s)
     * @param size can be square or rectangular
     * @param components number of elements per grid point: 1,2 or 4
     * @param doubled if true, create two underlying RenderTexture
     * @param depth if true, add a depth and stencil buffer
     */
    Buffer(const glm::vec2& size, unsigned components, bool doubled = false, bool depth = false);

    /**
     * @brief Run a shader on the buffer with several textures bound.
     * @param the return value of the Operator parenthesis operator
     * @return *this
     */
    Buffer& operator=(OperatorContext context);

    /**
     * @brief Make the texture linear.
     */
    void Linear();

    /**
     * @brief When sampling from outside the texture, a value of 0 is returned.
     */
    void ClampToEdge();

    /**
     * @brief Clears the Buffer
     */
    void Clear(const glm::vec4& colour) override;

    /**
     * @brief Renders an object on the front RenderTexture only
     * @param object the object to use
     * @param transform an optional transform to apply
     */
    void Render(Renderer::Drawable& object, const glm::mat4& transform = glm::mat4()) override;

    /**
     * @brief Clears the stencil on both RenderTexture
     */
    void ClearStencil();

    /**
     * @brief Swap the front and back RenderTexture
     * @return returns *this
     */
    Buffer& Swap();

    /**
     * @brief Returns a Sprite backed by the Texture of the front RenderTexture
     */
    Renderer::Sprite& Sprite();

    friend struct Back;
    friend struct Front;

    friend class Reader;
    friend class Writer;
private:
    void Add(const glm::vec2& size, unsigned components, bool depth);

    // of size 1 or 2
    std::vector<Renderer::RenderTexture> mTextures;
    Renderer::Sprite mSprite;
};

/**
 * @brief Helper class to Bind the Texture of the front RenderTexture of Buffer
 */
struct Front
{
    Front(Buffer& b) : buffer(b) {}

    void Bind(int n) { buffer.mTextures.front().Bind(n); }

    Buffer& buffer;
};

/**
 * @brief Helper class to Bind the Texture of the back RenderTexture of Buffer
 */
struct Back
{
    explicit Back(Buffer& b) : buffer(b) {}

    void Bind(int n) { buffer.mTextures.back().Bind(n); }

    Buffer& buffer;
};

}}

#endif /* Buffer_h */
