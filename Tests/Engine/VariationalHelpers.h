//
//  Common.h
//  Vortex2D
//

#include <vector>
#include <iostream>
#include <random>

#include <gmock/gmock.h>
#include <glm/vec2.hpp>

#include <Vortex2D/Renderer/Texture.h>
#include <Vortex2D/Renderer/Buffer.h>
#include <Vortex2D/Renderer/CommandBuffer.h>

#include <Vortex2D/Engine/LinearSolver/LinearSolver.h>

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
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<float> dist(0.0f, 1.0f);

    for(int i = 0; i < 16*sqr(size.x); ++i)
    {
        Vec2f pt(dist(gen), dist(gen));
        if (phi(pt) > 0 && pt[0] > 0.5)
        {
            sim.add_particle(pt);
        }
    }
}

static void SetVelocity(const glm::ivec2& size, Vortex2D::Renderer::Texture& buffer, FluidSim& sim)
{
    std::vector<glm::vec2> velocityData(size.x * size.y, glm::vec2(0.0f));
    for (int i = 0; i < size.x; i++)
    {
        for (int j = 0; j < size.y; j++)
        {
            std::size_t index = i + size.x * j;
            velocityData[index].x = sim.u(i, j);
            velocityData[index].y = sim.v(i, j);
        }
    }

    buffer.CopyFrom(velocityData);
}

static void SetSolidPhi(const glm::ivec2& size, Vortex2D::Renderer::Texture& buffer, FluidSim& sim)
{
    std::vector<float> phi(size.x * size.y, 0.0f);
    for (int i = 0; i < size.x; i++)
    {
        for (int j = 0; j < size.y; j++)
        {
            int width = size.x;
            phi[i + j * width] = width * sim.nodal_solid_phi(i, j);
        }
    }

    buffer.CopyFrom(phi);
}

static void SetLiquidPhi(const glm::ivec2& size, Vortex2D::Renderer::Texture& buffer, FluidSim& sim)
{
    std::vector<float> phi(size.x * size.y, 0.0f);
    for (int i = 0; i < size.x; i++)
    {
        for (int j = 0; j < size.y; j++)
        {
            int width = size.x;
            phi[i + j * width] = width * sim.liquid_phi(i, j);
        }
    }

    buffer.CopyFrom(phi);
}

static void BuildInputs(const Vortex2D::Renderer::Device& device, const glm::ivec2& size, FluidSim& sim, Vortex2D::Renderer::Texture& velocity, Vortex2D::Renderer::Texture& solidPhi, Vortex2D::Renderer::Texture& liquidPhi)
{
    //TODO do we need the copy here? can't we directly set the texture (create with host = true)?
    Vortex2D::Renderer::Texture inputVelocity(device, size.x, size.y, vk::Format::eR32G32Sfloat, true);
    SetVelocity(size, inputVelocity, sim);

    sim.project(0.01f);

    Vortex2D::Renderer::Texture inputSolidPhi(device, size.x, size.y, vk::Format::eR32Sfloat, true);
    SetSolidPhi(size, inputSolidPhi, sim);

    Vortex2D::Renderer::Texture inputLiquidPhi(device, size.x, size.y, vk::Format::eR32Sfloat, true);
    SetLiquidPhi(size, inputLiquidPhi, sim);

    Vortex2D::Renderer::ExecuteCommand(device, [&](vk::CommandBuffer commandBuffer)
    {
        velocity.CopyFrom(commandBuffer, inputVelocity);
        solidPhi.CopyFrom(commandBuffer, inputSolidPhi);
        liquidPhi.CopyFrom(commandBuffer, inputLiquidPhi);
    });
}

static void BuildLinearEquation(const glm::ivec2& size, Vortex2D::Renderer::Buffer& d, Vortex2D::Renderer::Buffer& l, Vortex2D::Renderer::Buffer& div, FluidSim& sim)
{
    std::vector<float> diagonalData(size.x * size.y);
    std::vector<glm::vec2> lowerData(size.x * size.y);
    std::vector<float> divData(size.x * size.y);

    for (int i = 1; i < size.x - 1; i++)
    {
        for (int j = 1; j < size.y - 1; j++)
        {
            unsigned index = i + size.x * j;
            divData[index] = (float)sim.rhs[index];
            diagonalData[index] = (float)sim.matrix(index, index);
            lowerData[index].x = (float)sim.matrix(index - 1, index);
            lowerData[index].y = (float)sim.matrix(index, index - size.x);
        }
    }

    d.CopyFrom(diagonalData);
    l.CopyFrom(lowerData);
    div.CopyFrom(divData);
}

static void PrintDiagonal(const glm::ivec2& size, Vortex2D::Renderer::Buffer& buffer)
{
    std::vector<float> pixels(size.x * size.y);
    buffer.CopyTo(pixels);

    for (std::size_t j = 0; j < size.y; j++)
    {
        for (std::size_t i = 0; i < size.x; i++)
        {
            std::size_t index = i + size.x * j;
            std::cout << "(" <<  pixels[index] << ")";
        }
        std::cout << std::endl;
    }
    std::cout << std::endl;
}

static void PrintWeights(const glm::ivec2& size, FluidSim& sim)
{
    for (int j = 1; j < size.y - 1; j++)
    {
        for (int i = 1; i < size.x - 1; i++)
        {
            int index = i + size.x * j;
            std::cout << "(" <<  sim.matrix(index + 1, index) << ","
                      << sim.matrix(index - 1, index) << ","
                      << sim.matrix(index, index + size.x) << ","
                      << sim.matrix(index, index - size.x) << ")";
        }
        std::cout << std::endl;
    }
    std::cout << std::endl;
}
