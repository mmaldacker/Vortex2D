//
//  Smoke.h
//  Vortex2D
//

#ifndef Vortex2D_Smoke_h
#define Vortex2D_Smoke_h

#include "BaseExample.h"

class SmokeExample : public BaseExample
{
public:
    SmokeExample()
        : BaseExample({glm::vec2(500), 1.0}, 0.033f)
        , source1(glm::vec2(20.0f)), source2(glm::vec2(20.0f))
        , force1(glm::vec2(20.0f)), force2(glm::vec2(20.0f))
        , density(dimensions)
    {
        source1.Position = force1.Position = {166.0f, 100.0f};
        source2.Position = force2.Position = {332.0f, 100.0f};

        force1.Colour = force2.Colour = {0.0f, 5.0f, 0.0f, 0.0f};

        source1.Colour = source2.Colour = gray;

        engine.RenderDirichlet(engine.TopBoundary);
        engine.RenderDirichlet(engine.BottomBoundary);
        engine.RenderDirichlet(engine.LeftBoundary);
        engine.RenderDirichlet(engine.RightBoundary);
        engine.ReinitialiseDirichlet();
    }

    void Frame() override
    {
        engine.RenderForce(force1);
        engine.RenderForce(force2);

        density.Render(source1);
        density.Render(source2);

        engine.Solve();
        density.Advect(engine);
    }

    void Render(Vortex2D::Renderer::RenderTarget & target) override
    {
        target.Render(density);
    }

private:
    Vortex2D::Renderer::Ellipse source1, source2;
    Vortex2D::Renderer::Ellipse force1, force2;
    Vortex2D::Fluid::Density density;
};

#endif
