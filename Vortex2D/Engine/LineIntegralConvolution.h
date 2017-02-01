//
//  LineIntegralConvolution.h
//  Vortex2D
//

#ifndef LineIntegralConvolution_h
#define LineIntegralConvolution_h

#include "Buffer.h"
#include "Operator.h"
#include "Drawable.h"
#include "Sprite.h"

namespace Vortex2D { namespace Fluid {

/**
 * @brief Represents a vector field using line integral convolution visualisation
 */
class LineIntegralConvolution : public Renderer::Drawable
{
public:
    LineIntegralConvolution(const glm::vec2& size);
    virtual ~LineIntegralConvolution();

    void Calculate(Renderer::Buffer & velocity);

    void Render(Renderer::RenderTarget& target, const glm::mat4& transform = glm::mat4()) override;

private:
    Renderer::Texture mWhiteNoise;
    Renderer::Operator mLic;
    Renderer::Buffer mOutput;
};

}}

#endif /* LineIntegralConvolution_h */
