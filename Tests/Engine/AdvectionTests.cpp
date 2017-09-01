//
//  AdvectionTests.cpp
//  Vortex2D
//

#include "Verify.h"
#include "VariationalHelpers.h"
#include <Vortex2D/Engine/Advection.h>

using namespace Vortex2D::Renderer;
using namespace Vortex2D::Fluid;

extern Device* device;

TEST(AdvectionTests, AdvectVelocity_Simple)
{
    glm::ivec2 size(50);

    FluidSim sim;
    sim.initialize(1.0f, size.x, size.y);
    sim.set_boundary(boundary_phi);

    AddParticles(size, sim, boundary_phi);

    sim.advance(0.01f);

    Texture output(*device, size.x, size.y, vk::Format::eR32G32Sfloat, true);
    Texture velocity(*device, size.x, size.y, vk::Format::eR32G32Sfloat, false);
    SetVelocity(size, output, sim);

    ExecuteCommand(*device, [&](vk::CommandBuffer commandBuffer)
    {
        velocity.CopyFrom(commandBuffer, output);
    });

    sim.advect(0.01f);

    Advection advection(*device, size, 0.01f, velocity);
    advection.AdvectVelocity();

    device->Queue().waitIdle();

    ExecuteCommand(*device, [&](vk::CommandBuffer commandBuffer)
    {
        output.CopyFrom(commandBuffer, velocity);
    });

    CheckVelocity(size, output, sim, 1e-5f);
}

TEST(AdvectionTests, AdvectVelocity_Complex)
{
    glm::ivec2 size(50);

    FluidSim sim;
    sim.initialize(1.0f, size.x, size.y);
    sim.set_boundary(complex_boundary_phi);

    AddParticles(size, sim, complex_boundary_phi);

    sim.add_force(0.01f);
    sim.advance(0.01f);

    Texture output(*device, size.x, size.y, vk::Format::eR32G32Sfloat, true);
    Texture velocity(*device, size.x, size.y, vk::Format::eR32G32Sfloat, false);
    SetVelocity(size, output, sim);

    ExecuteCommand(*device, [&](vk::CommandBuffer commandBuffer)
    {
        velocity.CopyFrom(commandBuffer, output);
    });

    sim.advect(0.01f);

    Advection advection(*device, size, 0.01f, velocity);
    advection.AdvectVelocity();

    device->Queue().waitIdle();

    ExecuteCommand(*device, [&](vk::CommandBuffer commandBuffer)
    {
        output.CopyFrom(commandBuffer, velocity);
    });

    CheckVelocity(size, output, sim, 1e-5f);
}

void PrintRGBA8(Texture& texture)
{
    std::vector<glm::u8vec4> pixels(texture.GetWidth() * texture.GetHeight());
    texture.CopyTo(pixels);

    for (uint32_t j = 0; j < texture.GetHeight(); j++)
    {
        for (uint32_t i = 0; i < texture.GetWidth(); i++)
        {
            uint8_t value = pixels[i + j * texture.GetWidth()].x;
            std::cout << "(" << (int)value << ")";
        }
        std::cout << std::endl;
    }
    std::cout << std::endl;
}

TEST(AdvectionTests, Advect)
{
    glm::ivec2 size(10);

    glm::vec2 vel(3.0f, 1.0f);
    glm::ivec2 pos(3, 4);

    Texture velocityInput(*device, size.x, size.y, vk::Format::eR32G32Sfloat, true);
    Texture velocity(*device, size.x, size.y, vk::Format::eR32G32Sfloat, false);

    std::vector<glm::vec2> velocityData(size.x * size.y, vel / glm::vec2(size));
    velocityInput.CopyFrom(velocityData);

    ExecuteCommand(*device, [&](vk::CommandBuffer commandBuffer)
    {
        velocity.CopyFrom(commandBuffer, velocityInput);
    });

    Texture fieldInput(*device, size.x, size.y, vk::Format::eB8G8R8A8Unorm, true);
    Texture field(*device, size.x, size.y, vk::Format::eB8G8R8A8Unorm, false);

    std::vector<glm::u8vec4> fieldData(size.x * size.y);
    fieldData[pos.x + size.x * pos.y].x = 128;
    fieldInput.CopyFrom(fieldData);

    ExecuteCommand(*device, [&](vk::CommandBuffer commandBuffer)
    {
        field.CopyFrom(commandBuffer, fieldInput);
    });

    Advection advection(*device, size, 1.0f, velocity);
    advection.AdvectInit(field);
    advection.Advect();

    device->Handle().waitIdle();

    ExecuteCommand(*device, [&](vk::CommandBuffer commandBuffer)
    {
        fieldInput.CopyFrom(commandBuffer, field);
    });

    std::vector<glm::u8vec4> pixels(fieldInput.GetWidth() * fieldInput.GetHeight());
    fieldInput.CopyTo(pixels);

    pos += glm::ivec2(vel);
    ASSERT_EQ(128, pixels[pos.x + size.x * pos.y].x);
}
