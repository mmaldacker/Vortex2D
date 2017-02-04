//
//  Common.h
//  Vortex2D
//

#include <vector>
#include <iostream>

#include "gtest/gtest.h"
#include "LinearSolver.h"
#include "Disable.h"
#include "RenderTexture.h"
#include "Operator.h"
#include "Reader.h"
#include "variationalplusgfm/fluidsim.h"

using namespace Vortex2D::Renderer;
using namespace Vortex2D::Fluid;

template<typename T>
void PrintData(int width, int height, const std::vector<T>& data)
{
    for (int j = 0; j < height; j++)
    {
        for (int i = 0; i < width; i++)
        {
            std::cout << "(" << data[i + j * width] << ")";
        }
        std::cout << std::endl;
    }
    std::cout << std::endl;
}

static void CheckTexture(int width, int height, const std::vector<float>& data, Reader& reader)
{
    reader.Read();

    for (int i = 0; i < width; i++)
    {
        for (int j = 0; j < height; j++)
        {
            float value = data[i + j * width];
            EXPECT_FLOAT_EQ(value, reader.GetFloat(i, j)) << "Value not equal at " << i << ", " << j;
        }
    }
}

static void CheckTexture(int width, int height, const std::vector<float>& data, Buffer& buffer)
{
    Reader reader(buffer);
    CheckTexture(width, height, data, reader);
}

static void CheckTexture(int width, int height, const std::vector<float>& data, RenderTexture& texture)
{
    Reader reader(texture);
    CheckTexture(width, height, data, reader);
}

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

static void AddParticles(const glm::vec2& size, FluidSim& sim)
{
    for(int i = 0; i < 4*sqr(size.x); ++i)
    {
        float x = randhashf(i*2, 0,1);
        float y = randhashf(i*2+1, 0,1);
        Vec2f pt(x,y);
        if (boundary_phi(pt) > 0 && pt[0] > 0.5)
        {
            sim.add_particle(pt);
        }
    }
}

class EmptyLinearSolver : public LinearSolver
{
public:
    void Init(Data& data) override
    {

    }

    void Solve(Data& data, Parameters& params) override
    {

    }
};


