//
//  Common.h
//  Vortex2D
//

#include <vector>
#include <iostream>

#include <gmock/gmock.h>
#include <glm/vec2.hpp>

#include "variationalplusgfm/fluidsim.h"

//Boundary definition - several circles in a circular domain.

const Vec2f c0(0.5f,0.5f), c1(0.7f,0.5f), c2(0.3f,0.35f), c3(0.5f,0.7f);
const float rad0 = 0.4f,  rad1 = 0.1f,  rad2 = 0.1f,   rad3 = 0.1f;

static float circle_phi(const Vec2f& position, const Vec2f& centre, float radius)
{
    return (dist(position,centre) - radius);
}

static float boundary_phi(const Vec2f& position)
{
    float phi0 = -circle_phi(position, c0, rad0);

    return phi0;
}

static float complex_boundary_phi(const Vec2f& position)
{
    float phi0 = -circle_phi(position, c0, rad0);
    float phi1 = circle_phi(position, c1, rad1);
    float phi2 = circle_phi(position, c2, rad2);
    float phi3 = circle_phi(position, c3, rad3);

    return min(min(phi0,phi1),min(phi2,phi3));
}

static void AddParticles(const glm::vec2& size, FluidSim& sim, float (*phi)(const Vec2f&))
{
    for(int i = 0; i < 4*sqr(size.x); ++i)
    {
        float x = randhashf(i*2, 0,1);
        float y = randhashf(i*2+1, 0,1);
        Vec2f pt(x,y);
        if (phi(pt) > 0 && pt[0] > 0.5)
        {
            sim.add_particle(pt);
        }
    }
}

/*
static void SetVelocity(Buffer& buffer, FluidSim& sim)
{
    std::vector<glm::vec2> velocityData(buffer.Width() * buffer.Height(), glm::vec2(0.0f));
    for (int i = 0; i < buffer.Width(); i++)
    {
        for (int j = 0; j < buffer.Height(); j++)
        {
            std::size_t index = i + buffer.Width() * j;
            velocityData[index].x = sim.u(i, j);
            velocityData[index].y = sim.v(i, j);
        }
    }

    Writer(buffer).Write(velocityData);
}

static void SetSolidPhi(Buffer& buffer, FluidSim& sim)
{
    std::vector<float> phi(buffer.Width() * buffer.Height(), 0.0f);
    for (int i = 0; i < buffer.Width(); i++)
    {
        for (int j = 0; j < buffer.Height(); j++)
        {
            phi[i + j * buffer.Width()] = sim.nodal_solid_phi(i/2, j/2);
        }
    }

    Writer(buffer).Write(phi);
}

static void SetLiquidPhi(Buffer& buffer, FluidSim& sim)
{
    std::vector<float> phi(buffer.Width() * buffer.Height(), 0.0f);
    for (int i = 0; i < buffer.Width(); i++)
    {
        for (int j = 0; j < buffer.Height(); j++)
        {
            phi[i + j * buffer.Width()] = sim.liquid_phi(i, j);
        }
    }

    Writer(buffer).Write(phi);
}
*/
