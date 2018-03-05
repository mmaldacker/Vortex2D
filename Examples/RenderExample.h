//
//  RenderExample.h
//  Vortex2D
//

#include <Vortex2D/Vortex2D.h>

#include "Runner.h"

class RenderExample : public Runner
{
public:
    RenderExample(const Vortex2D::Renderer::Device& device, const glm::vec2& /*size*/)
        : rectangle(device, {100,100}, {1.0f, 1.0f, 1.0f, 1.0f})
        , circle(device, glm::vec2(50), glm::vec4(1.0f))
    {
        rectangle.Position = {200, 200};
        rectangle.Rotation = 45.0f;

        circle.Position = {500, 500};
    }

    void Init(const Vortex2D::Renderer::Device& device,
              Vortex2D::Renderer::RenderTarget& renderTarget) override
    {
        auto blendMode = vk::PipelineColorBlendAttachmentState()
                .setBlendEnable(true)
                .setAlphaBlendOp(vk::BlendOp::eAdd)
                .setColorBlendOp(vk::BlendOp::eAdd)
                .setSrcColorBlendFactor(vk::BlendFactor::eSrcAlpha)
                .setSrcAlphaBlendFactor(vk::BlendFactor::eOne)
                .setDstColorBlendFactor(vk::BlendFactor::eOneMinusSrcAlpha)
                .setDstAlphaBlendFactor(vk::BlendFactor::eZero);

        render = renderTarget.Record({rectangle, circle}, blendMode);
    }

    void Step() override
    {
        render.Submit();
    }

private:
    Vortex2D::Renderer::Rectangle rectangle;
    Vortex2D::Renderer::Ellipse circle;
    Vortex2D::Renderer::RenderCommand render;
};
