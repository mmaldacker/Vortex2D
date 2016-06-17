//
//  VelocitySmokeTest.h
//  Vertex2D
//

#ifndef Vertex2D_VelocitySmokeTest_h
#define Vertex2D_VelocitySmokeTest_h


#include "BaseExample.h"
#include "Shapes.h"
#include "Density.h"

class VelocitySmokeExample : public BaseExample
{
public:
    VelocitySmokeExample()
    : BaseExample({glm::vec2(500), 1.0}, 0.033)
    , obstacle({50.0f, 50.0f}), force({50.0f, 50.0f})
    , top({500,1}), bottom({500,1})
    , left({1,500}), right({1,500})
    , density(dimensions)
    {
        obstacle.Position = {200.0f, 50.0f};

        force.Position = (glm::vec2)obstacle.Position;
        force.Colour = {0.0f, speed/0.033f, 0.0f, 0.0f};

        top.Colour = bottom.Colour = left.Colour = right.Colour = glm::vec4{1.0f};

        top.Position = {0.0f, 0.0f};
        bottom.Position = {0.0f, 499.0f};
        left.Position = {0.0f, 0.0f};
        right.Position = {499.0f, 0.0f};

        Vortex2D::Renderer::Rectangle source({400.0f, 50.0f});
        source.Position = {50.0f, 200.0f};
        source.Colour = gray;
        density.Render(source);
    }

    void Frame() override
    {
        engine.ClearBoundaries();
        engine.ClearVelocities();
        
        engine.RenderNeumann(top);
        engine.RenderNeumann(bottom);
        engine.RenderNeumann(left);
        engine.RenderNeumann(right);

        obstacle.Colour = glm::vec4(1.0);
        engine.RenderNeumann(obstacle);

        glm::vec2 pos = obstacle.Position;
        if(pos.y < 400.0f)
        {
            obstacle.Position = force.Position = pos + glm::vec2{0.0f,speed};
            engine.RenderVelocities(force);
        }

        engine.Solve();
        density.Advect(engine);
    }

    void Render(Vortex2D::Renderer::RenderTarget & target) override
    {
        target.Render(density);
        obstacle.Colour = green;
        target.Render(obstacle);
    }

private:
    Vortex2D::Renderer::Rectangle obstacle, force;
    Vortex2D::Renderer::Rectangle top, bottom, left, right;
    Vortex2D::Fluid::Density density;

    const float speed = 2.0f;
};


#endif
