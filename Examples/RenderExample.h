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
  RenderExample(const Vortex::Renderer::Device& device, const glm::vec2& /*size*/)
      : rectangle(device, {25, 25}), circle(device, glm::vec2(10))
  {
    rectangle.Position = {50, 50};
    rectangle.Rotation = 45.0f;
    rectangle.Colour = green;

    circle.Position = {100, 100};
    circle.Colour = green;
  }

  void Init(const Vortex::Renderer::Device& device,
            Vortex::Renderer::RenderTarget& renderTarget) override
  {
    Vortex::Renderer::ColorBlendState blendState;
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
  Vortex::Renderer::Rectangle rectangle;
  Vortex::Renderer::Ellipse circle;
  Vortex::Renderer::RenderCommand render;
};
