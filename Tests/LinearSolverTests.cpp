//
//  LinearSolverTests.cpp
//  Vortex2D
//

#include "gtest/gtest.h"
#include "Writer.h"
#include "ConjugateGradient.h"
#include "variationalplusgfm/fluidsim.h"

using namespace Vortex2D::Renderer;
using namespace Vortex2D::Fluid;

//Boundary definition - several circles in a circular domain.

Vec2f c0(0.5,0.5), c1(0.7,0.5), c2(0.3,0.35), c3(0.5,0.7);
float rad0 = 0.4,  rad1 = 0.1,  rad2 = 0.1,   rad3 = 0.1;

float circle_phi(const Vec2f& position, const Vec2f& centre, float radius)
{
   return (dist(position,centre) - radius);
}

float boundary_phi(const Vec2f& position)
{
   float phi0 = -circle_phi(position, c0, rad0);
   float phi1 = circle_phi(position, c1, rad1);
   float phi2 = circle_phi(position, c2, rad2);
   float phi3 = circle_phi(position, c3, rad3);

   return phi0;//min(min(phi0,phi1),min(phi2,phi3));
}

TEST(LinearSolverTests, Simple)
{
    ConjugateGradient solver(glm::vec2(50));

    FluidSim sim;
    sim.initialize(1.0f, 50, 50);
    sim.set_boundary(boundary_phi);

    for(int i = 0; i < sqr(50); ++i)
    {
       float x = randhashf(i*2, 0,1);
       float y = randhashf(i*2+1, 0,1);
       Vec2f pt(x,y);
       if (boundary_phi(pt) > 0 && pt[0] > 0.5)
       {
          sim.add_particle(pt);
       }
    }

    sim.project(0.033f);

    LinearSolver::Data data(glm::vec2(50.0f));

    Writer pressureWriter(data.Pressure);
    std::vector<glm::vec4> pressureData;
    Writer diagonalWriter(data.Diagonal);
}
