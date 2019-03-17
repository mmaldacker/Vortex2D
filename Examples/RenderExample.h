//
//  RenderExample.h
//  Vortex2D
//

#include <Vortex2D/Vortex2D.h>

#include "Runner.h"

extern glm::vec4 green;

class RenderExample : public Runner
{
public:
  RenderExample(const Vortex2D::Renderer::Device& device, const glm::vec2& /*size*/)
      : rectangle(device, {25, 25}), circle(device, glm::vec2(10))
  {
    rectangle.Position = {50, 50};
    rectangle.Rotation = 45.0f;
    rectangle.Colour = green;

    circle.Position = {100, 100};
    circle.Colour = green;
  }

  void Init(const Vortex2D::Renderer::Device& device,
            Vortex2D::Renderer::RenderTarget& renderTarget) override
  {
    Vortex2D::Renderer::ColorBlendState blendState;
    blendState.ColorBlend.setBlendEnable(true)
        .setAlphaBlendOp(vk::BlendOp::eAdd)
        .setColorBlendOp(vk::BlendOp::eAdd)
        .setSrcColorBlendFactor(vk::BlendFactor::eSrcAlpha)
        .setSrcAlphaBlendFactor(vk::BlendFactor::eOne)
        .setDstColorBlendFactor(vk::BlendFactor::eOneMinusSrcAlpha)
        .setDstAlphaBlendFactor(vk::BlendFactor::eZero);

    render = renderTarget.Record({rectangle, circle}, blendState);
  }

  void Step() override { render.Submit(); }

private:
  Vortex2D::Renderer::Rectangle rectangle;
  Vortex2D::Renderer::Ellipse circle;
  Vortex2D::Renderer::RenderCommand render;
};
