//
//  Common.h
//  Vortex2D
//

#include <vector>
#include <iostream>

#include <gmock/gmock.h>

#include <Vortex2D/Renderer/Data.h>
#include <Vortex2D/Renderer/Operator.h>
#include <Vortex2D/Renderer/Reader.h>
#include <Vortex2D/Renderer/Writer.h>

#include <Vortex2D/Engine/LinearSolver/LinearSolver.h>

#include "variationalplusgfm/fluidsim.h"

using namespace Vortex2D::Renderer;
using namespace Vortex2D::Fluid;

// FIXME move implementations in .cpp file

static void PrintData(float data)
{
    std::cout << "(" << data << ")";
}

template<typename T>
void PrintData(int width, int height, const std::vector<T>& data)
{
    for (int j = 0; j < height; j++)
    {
        for (int i = 0; i < width; i++)
        {
            PrintData(data[i + j * width]);
        }
        std::cout << std::endl;
    }
    std::cout << std::endl;
}

static void PrintVelocity(const glm::vec2& size, FluidSim& sim)
{
    for (std::size_t j = 0; j < size.y; j++)
    {
        for (std::size_t i = 0; i < size.x; i++)
        {
            std::cout << "(" << sim.u(i, j) << "," << sim.v(i, j) << ")";
        }
        std::cout << std::endl;
    }
    std::cout << std::endl;
}

template<typename T>
static void DrawSquare(int width, int height, std::vector<T>& data, const glm::vec2& centre, const glm::vec2& size, T value)
{
    for (int i = 0; i < size.x; i++)
    {
        for (int j = 0; j < size.y; j++)
        {
            int x = i + centre.x;
            int y = j + centre.y;
            data[x + y * width] = value;
        }
    }
}

static void CheckData(int i, int j, float value, Reader& reader)
{
    EXPECT_FLOAT_EQ(value, reader.GetFloat(i, j)) << "Value not equal at " << i << ", " << j;

}

static void CheckData(int i, int j, const glm::vec4& value, Reader& reader)
{
    EXPECT_FLOAT_EQ(value.x, reader.GetVec4(i, j).x) << "Value not equal at " << i << ", " << j;
    EXPECT_FLOAT_EQ(value.y, reader.GetVec4(i, j).y) << "Value not equal at " << i << ", " << j;
    EXPECT_FLOAT_EQ(value.z, reader.GetVec4(i, j).z) << "Value not equal at " << i << ", " << j;
    EXPECT_FLOAT_EQ(value.w, reader.GetVec4(i, j).w) << "Value not equal at " << i << ", " << j;
}

template<typename T>
static void CheckTexture(const std::vector<T>& data, Reader& reader)
{
    reader.Read();

    for (int i = 0; i < reader.Width(); i++)
    {
        for (int j = 0; j < reader.Height(); j++)
        {
            T value = data[i + j * reader.Width()];
            CheckData(i, j, value, reader);
        }
    }
}

template<typename T>
static void CheckTexture(const std::vector<T>& data, Buffer& buffer)
{
    Reader reader(buffer);
    CheckTexture(data, reader);
}

template<typename T>
static void CheckTexture(const std::vector<T>& data, RenderTexture& texture)
{
    Reader reader(texture);
    CheckTexture(data, reader);
}

static void CheckVelocity(Buffer& buffer, FluidSim& sim, float error = 1e-6)
{
    Reader reader(buffer);
    reader.Read();

    // FIXME need to check the entire velocity buffer
    for (std::size_t i = 1; i < buffer.Width() - 1; i++)
    {
        for (std::size_t j = 1; j < buffer.Height() - 1; j++)
        {
            EXPECT_NEAR(sim.u(i, j), reader.GetVec2(i, j).x, error) << "Mismatch at " << i << "," << j;
            EXPECT_NEAR(sim.v(i, j), reader.GetVec2(i, j).y, error) << "Mismatch at " << i << "," << j;
        }
    }
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

class MockLinearSolver : public LinearSolver
{
public:

    MOCK_METHOD5(Build, void(Data&, Operator&, Operator&, Buffer&, Buffer&));
    MOCK_METHOD1(Init, void(Data&));
    MOCK_METHOD2(Solve, void(Data&, Parameters&));
};


