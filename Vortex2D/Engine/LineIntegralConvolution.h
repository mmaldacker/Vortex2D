//
//  LineIntegralConvolution.h
//  Vortex2D
//

#ifndef LineIntegralConvolution_h
#define LineIntegralConvolution_h

#include <Vortex2D/Renderer/Data.h>
#include <Vortex2D/Renderer/Operator.h>
#include <Vortex2D/Renderer/Drawable.h>
#include <Vortex2D/Renderer/Sprite.h>

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

    void Render(const Renderer::Device& device, Renderer::RenderTarget & target) override;

private:
    Renderer::Texture mWhiteNoise;
    Renderer::Operator mLic;
    Renderer::Buffer mOutput;
};

}}

#endif /* LineIntegralConvolution_h */
