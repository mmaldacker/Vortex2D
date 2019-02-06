//
//  VariationalHelpers.cpp
//  Vortex2D
//

#include "VariationalHelpers.h"

float circle_phi(const Vec2f& position, const Vec2f& centre, float radius)
{
    return (dist(position,centre) - radius);
}

float boundary_phi(const Vec2f& position)
{
    float phi0 = -circle_phi(position, c0, rad0);

    return phi0;
}

float complex_boundary_phi(const Vec2f& position)
{
    float phi0 = -circle_phi(position, c0, rad0);
    float phi1 = circle_phi(position, c1, rad1);
    float phi2 = circle_phi(position, c2, rad2);
    float phi3 = circle_phi(position, c3, rad3);

    return min(min(phi0,phi1),min(phi2,phi3));
}

void AddParticles(const glm::ivec2& size, FluidSim& sim, float (*phi)(const Vec2f&))
{
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<float> dist(0.0f, 1.0f);

    std::vector<int> count(size.x*size.y);
    for(int i = 0; i < 4*sqr(size.x); ++i)
    {
        Vec2f pt(dist(gen), dist(gen));
        if (phi(pt) > 0 && pt[0] > 0.5)
        {
            int index = int(pt[0] * size.x) + int(pt[1] * size.y) * size.x;
            if (count[index] < 8)
            {
                sim.add_particle(pt);
                count[index]++;
            }
        }
    }
}

void SetVelocity(const Vortex2D::Renderer::Device& device,
                        const glm::ivec2& size,
                        Vortex2D::Fluid::Velocity& velocity,
                        FluidSim& sim)
{
    Vortex2D::Renderer::Texture input(device, size.x, size.y, vk::Format::eR32G32Sfloat, VMA_MEMORY_USAGE_CPU_ONLY);

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

    input.CopyFrom(velocityData);
    ExecuteCommand(device, [&](vk::CommandBuffer commandBuffer)
    {
        velocity.CopyFrom(commandBuffer, input);
    });
}

void SetSolidPhi(const Vortex2D::Renderer::Device& device,
                        const glm::ivec2& size,
                        Vortex2D::Renderer::Texture& solidPhi,
                        FluidSim& sim,
                        float scale)
{
    Vortex2D::Renderer::Texture input(device, size.x, size.y, vk::Format::eR32Sfloat, VMA_MEMORY_USAGE_CPU_ONLY);

    std::vector<float> phi(size.x * size.y, 0.0f);
    for (int i = 0; i < size.x; i++)
    {
        for (int j = 0; j < size.y; j++)
        {
            int width = size.x;
            phi[i + j * width] = scale * sim.nodal_solid_phi(i, j);
        }
    }

    input.CopyFrom(phi);
    ExecuteCommand(device, [&](vk::CommandBuffer commandBuffer)
    {
        solidPhi.CopyFrom(commandBuffer, input);
    });
}

void SetLiquidPhi(const Vortex2D::Renderer::Device& device,
                         const glm::ivec2& size,
                         Vortex2D::Renderer::Texture& liquidPhi,
                         FluidSim& sim,
                         float scale)
{
    Vortex2D::Renderer::Texture input(device, size.x, size.y, vk::Format::eR32Sfloat, VMA_MEMORY_USAGE_CPU_ONLY);

    std::vector<float> phi(size.x * size.y, 0.0f);
    for (int i = 0; i < size.x; i++)
    {
        for (int j = 0; j < size.y; j++)
        {
            int width = size.x;
            phi[i + j * width] = scale * sim.liquid_phi(i, j);
        }
    }

    input.CopyFrom(phi);
    ExecuteCommand(device, [&](vk::CommandBuffer commandBuffer)
    {
        liquidPhi.CopyFrom(commandBuffer, input);
    });
}

void BuildInputs(const Vortex2D::Renderer::Device& device,
                        const glm::ivec2& size,
                        FluidSim& sim,
                        Vortex2D::Fluid::Velocity& velocity,
                        Vortex2D::Renderer::Texture& solidPhi,
                        Vortex2D::Renderer::Texture& liquidPhi)
{
    SetVelocity(device, size, velocity, sim);

    sim.compute_phi();
    sim.extrapolate_phi();
    sim.compute_pressure_weights();
    sim.solve_pressure(0.01f);

    SetSolidPhi(device, size, solidPhi, sim);
    SetLiquidPhi(device, size, liquidPhi, sim);
}

void BuildLinearEquation(const glm::ivec2& size,
                                Vortex2D::Renderer::Buffer<float>& d,
                                Vortex2D::Renderer::Buffer<glm::vec2>& l,
                                Vortex2D::Renderer::Buffer<float>& div, FluidSim& sim)
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

    using Vortex2D::Renderer::CopyFrom;

    CopyFrom(d, diagonalData);
    CopyFrom(l, lowerData);
    CopyFrom(div, divData);
}

void PrintDiagonal(const glm::ivec2& size, Vortex2D::Renderer::Buffer<float>& buffer)
{
    std::vector<float> pixels(size.x * size.y);
    Vortex2D::Renderer::CopyTo(buffer, pixels);

    for (int j = 0; j < size.y; j++)
    {
        for (int i = 0; i < size.x; i++)
        {
            int index = i + size.x * j;
            std::cout << "(" <<  pixels[index] << ")";
        }
        std::cout << std::endl;
    }
    std::cout << std::endl;
}

void PrintWeights(const glm::ivec2& size, FluidSim& sim)
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

void PrintVelocity(const Vortex2D::Renderer::Device& device, const glm::ivec2& size, Vortex2D::Renderer::Texture& velocity)
{
    Vortex2D::Renderer::Texture output(device, size.x, size.y, vk::Format::eR32G32Sfloat, VMA_MEMORY_USAGE_CPU_ONLY);
    ExecuteCommand(device, [&](vk::CommandBuffer commandBuffer)
    {
        output.CopyFrom(commandBuffer, velocity);
    });

    std::vector<glm::vec2> pixels(size.x * size.y);
    output.CopyTo(pixels);

    for (int j = 0; j < size.y; j++)
    {
        for (int i = 0; i < size.x; i++)
        {
            int index = i + size.x * j;
            std::cout << "(" <<  pixels[index].x << "," << pixels[index].y << ")";
        }
        std::cout << std::endl;
    }
    std::cout << std::endl;
}

void PrintVelocity(const glm::ivec2& size, FluidSim& sim)
{
    for (int j = 0; j < size.y; j++)
    {
        for (int i = 0; i < size.x; i++)
        {
            std::cout << "(" << sim.u(i, j) << "," << sim.v(i, j) << ")";
        }
        std::cout << std::endl;
    }
    std::cout << std::endl;
}

void CheckVelocity(const Vortex2D::Renderer::Device& device,
                   const glm::ivec2& size,
                   Vortex2D::Fluid::Velocity& velocity,
                   FluidSim& sim,
                   float error)
{
    Vortex2D::Renderer::Texture output(device, size.x, size.y, vk::Format::eR32G32Sfloat, VMA_MEMORY_USAGE_CPU_ONLY);
    ExecuteCommand(device, [&](vk::CommandBuffer commandBuffer)
    {
        output.CopyFrom(commandBuffer, velocity);
    });

    std::vector<glm::vec2> pixels(size.x * size.y);
    output.CopyTo(pixels);

    // TODO need to check the entire velocity buffer
    for (int i = 1; i < size.x - 1; i++)
    {
        for (int j = 1; j < size.y - 1; j++)
        {
            auto uv = pixels[i + j * size.x];
            EXPECT_NEAR(sim.u(i, j), uv.x, error) << "Mismatch at " << i << "," << j;
            EXPECT_NEAR(sim.v(i, j), uv.y, error) << "Mismatch at " << i << "," << j;
        }
    }
}

void CheckVelocity(const Vortex2D::Renderer::Device& device,
                   const glm::ivec2& size,
                   Vortex2D::Renderer::Texture& velocity,
                   const std::vector<glm::vec2>& velocityData,
                   float error)
{
    assert(velocityData.size() == size.x * size.y);

    Vortex2D::Renderer::Texture output(device, size.x, size.y, vk::Format::eR32G32Sfloat, VMA_MEMORY_USAGE_CPU_ONLY);
    ExecuteCommand(device, [&](vk::CommandBuffer commandBuffer)
    {
        output.CopyFrom(commandBuffer, velocity);
    });

    std::vector<glm::vec2> pixels(size.x * size.y);
    output.CopyTo(pixels);

    // TODO need to check the entire velocity buffer
    for (int i = 1; i < size.x - 1; i++)
    {
        for (int j = 1; j < size.y - 1; j++)
        {
            auto index = i + j * size.x;
            auto uv = pixels[index];
            auto expectedUv = velocityData[index];
            EXPECT_NEAR(expectedUv.x, uv.x, error) << "Mismatch at " << i << "," << j;
            EXPECT_NEAR(expectedUv.y, uv.y, error) << "Mismatch at " << i << "," << j;
        }
    }
}

void CheckValid(const glm::ivec2& size, FluidSim& sim, Vortex2D::Renderer::Buffer<glm::ivec2>& valid)
{
    std::vector<glm::ivec2> validData(size.x*size.y);
    Vortex2D::Renderer::CopyTo(valid, validData);

    for (int i = 0; i < size.x - 1; i++)
    {
        for (int j = 0; j < size.y - 1; j++)
        {
            std::size_t index = i + j * size.x;
            EXPECT_EQ(validData[index].x, sim.u_valid(i, j)) << "Mismatch at " << i << "," << j;
            EXPECT_EQ(validData[index].y, sim.v_valid(i, j)) << "Mismatch at " << i << "," << j;
        }
    }
}

void CheckDiv(const glm::ivec2& size, Vortex2D::Renderer::Buffer<float>& buffer, FluidSim& sim, float error)
{
    std::vector<float> pixels(size.x * size.y);
    Vortex2D::Renderer::CopyTo(buffer, pixels);

    for (int i = 0; i < size.x; i++)
    {
        for (int j = 0; j < size.y; j++)
        {
            std::size_t index = i + size.x * j;
            EXPECT_NEAR(sim.rhs[index], pixels[index], error) << "Mismatch at (" << i << "," << j << ")";
        }
    }
}

void PrintDiv(const glm::ivec2& size, FluidSim& sim)
{
    for (int j = 0; j < size.y; j++)
    {
        for (int i = 0; i < size.x; i++)
        {
            int index = i + size.x * j;
            std::cout << "(" <<  sim.rhs[index] << ")";
        }
        std::cout << std::endl;
    }

    std::cout << std::endl;
}

void CheckPressure(const glm::ivec2& size, const std::vector<double>& pressure, Vortex2D::Renderer::Buffer<float>& bufferPressure, float error)
{
    std::vector<float> bufferPressureData(size.x * size.y);
    Vortex2D::Renderer::CopyTo(bufferPressure, bufferPressureData);

    for (int i = 0; i < size.x; i++)
    {
        for (int j = 0; j < size.y; j++)
        {
            std::size_t index = i + j * size.x;
            float value = (float)pressure[index];
            EXPECT_NEAR(value, bufferPressureData[index], error) << "Mismatch at " << i << ", " << j << "\n";
        }
    }
}
