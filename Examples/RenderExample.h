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
  RenderExample(Vortex::Renderer::Device& device, const glm::vec2& size)
      : renderCircle(device, 50, 50, Vortex::Renderer::Format::R32Sfloat)
      , contour(device, renderCircle, glm::ivec2(50))
  {
  }

  void Init(Vortex::Renderer::Device& device, Vortex::Renderer::RenderTarget& renderTarget) override
  {
    {
      auto circle = std::make_shared<Vortex::Fluid::Circle>(device, 10.0f);
      circle->Position = {25., 25.};

      renderCircle.Record({circle}).Submit().Wait();
      contour.Generate();
    }

    auto rectangle = std::make_shared<Vortex::Renderer::Rectangle>(device, glm::vec2{25, 25});
    rectangle->Position = {50, 50};
    rectangle->Rotation = 45.0f;
    rectangle->Colour = green;

    auto circle = std::make_shared<Vortex::Renderer::Ellipse>(device, glm::vec2(10));
    circle->Position = {100, 100};
    circle->Colour = green;

    auto mesh = std::make_shared<Vortex::Renderer::Mesh>(
        device, contour.GetVertices(), contour.GetIndices(), contour.GetDrawParameters());
    mesh->Position = {100, 100};
    mesh->Scale = {3., 3.};
    mesh->Colour = green;

    Vortex::Renderer::ColorBlendState blendState(Vortex::Renderer::BlendFactor::SrcAlpha,
                                                 Vortex::Renderer::BlendFactor::OneMinusSrcAlpha,
                                                 Vortex::Renderer::BlendOp::Add,
                                                 Vortex::Renderer::BlendFactor::One,
                                                 Vortex::Renderer::BlendFactor::Zero,
                                                 Vortex::Renderer::BlendOp::Add);

    render = renderTarget.Record({rectangle, circle, mesh}, blendState);
  }

  void Step() override { render.Submit(); }

private:
  Vortex::Renderer::RenderTexture renderCircle;
  Vortex::Fluid::Contour contour;
  Vortex::Renderer::RenderCommand render;
};
