//
//  RenderExample.h
//  Vortex
//

#pragma once

#include <Vortex/Vortex.h>

#include "Runner.h"

extern glm::vec4 green;

class RenderExample : public Runner
{
public:
  RenderExample(const Vortex::Renderer::Device& device, const glm::vec2& size)
      : rectangle(device, {25, 25})
      , circle(device, glm::vec2(10))
      , renderCircle(device, size.x, size.y, vk::Format::eR32Sfloat)
      , contour(device, renderCircle, size)
      , mesh(device, contour.GetVertices(), contour.GetIndices(), contour.GetDrawParameters())
  {
    rectangle.Position = {50, 50};
    rectangle.Rotation = 45.0f;
    rectangle.Colour = green;

    circle.Position = {100, 100};
    circle.Colour = green;

    mesh.Position = {0, 0};
    mesh.Colour = green;
  }

  void Init(const Vortex::Renderer::Device& device,
            Vortex::Renderer::RenderTarget& renderTarget) override
  {
    {
      Vortex::Fluid::Circle circle(device, 30.0f);
      circle.Position = {175, 175};

      renderCircle.Record({circle}).Submit().Wait();
      contour.Generate();
    }

    Vortex::Renderer::ColorBlendState blendState;
    blendState.ColorBlend.setBlendEnable(true)
        .setAlphaBlendOp(vk::BlendOp::eAdd)
        .setColorBlendOp(vk::BlendOp::eAdd)
        .setSrcColorBlendFactor(vk::BlendFactor::eSrcAlpha)
        .setSrcAlphaBlendFactor(vk::BlendFactor::eOne)
        .setDstColorBlendFactor(vk::BlendFactor::eOneMinusSrcAlpha)
        .setDstAlphaBlendFactor(vk::BlendFactor::eZero);

    render = renderTarget.Record({rectangle, circle, mesh}, blendState);
  }

  void Step() override { render.Submit(); }

private:
  Vortex::Renderer::Rectangle rectangle;
  Vortex::Renderer::Ellipse circle;
  Vortex::Renderer::RenderTexture renderCircle;
  Vortex::Fluid::Contour contour;
  Vortex::Renderer::Mesh mesh;
  Vortex::Renderer::RenderCommand render;
};
