//
//  Common.h
//  Vortex2D
//

#include <vector>
#include <iostream>
#include <random>

#include <gmock/gmock.h>

#include <Vortex2D/Renderer/Texture.h>
#include <Vortex2D/Renderer/Buffer.h>
#include <Vortex2D/Renderer/CommandBuffer.h>

#include <Vortex2D/Engine/LinearSolver/LinearSolver.h>
#include <Vortex2D/Engine/Velocity.h>

#include "fluidsim.h"

//Boundary definition - several circles in a circular domain.

const Vec2f c0(0.5f,0.5f), c1(0.7f,0.5f), c2(0.3f,0.35f), c3(0.5f,0.7f);
const float rad0 = 0.4f,  rad1 = 0.1f,  rad2 = 0.1f,   rad3 = 0.1f;

float circle_phi(const Vec2f& position, const Vec2f& centre, float radius);

float boundary_phi(const Vec2f& position);

float complex_boundary_phi(const Vec2f& position);

void AddParticles(const glm::ivec2& size, FluidSim& sim, float (*phi)(const Vec2f&));
void SetVelocity(const Vortex2D::Renderer::Device& device,
                        const glm::ivec2& size,
                        Vortex2D::Fluid::Velocity& velocity,
                        FluidSim& sim);

void SetSolidPhi(const Vortex2D::Renderer::Device& device,
                        const glm::ivec2& size,
                        Vortex2D::Renderer::Texture& solidPhi,
                        FluidSim& sim,
                        float scale = 1.0f);
void SetLiquidPhi(const Vortex2D::Renderer::Device& device,
                         const glm::ivec2& size,
                         Vortex2D::Renderer::Texture& liquidPhi,
                         FluidSim& sim,
                         float scale = 1.0f);

void BuildInputs(const Vortex2D::Renderer::Device& device,
                        const glm::ivec2& size,
                        FluidSim& sim,
                        Vortex2D::Fluid::Velocity& velocity,
                        Vortex2D::Renderer::Texture& solidPhi,
                        Vortex2D::Renderer::Texture& liquidPhi);

void BuildLinearEquation(const glm::ivec2& size,
                                Vortex2D::Renderer::Buffer<float>& d,
                                Vortex2D::Renderer::Buffer<glm::vec2>& l,
                                Vortex2D::Renderer::Buffer<float>& div, FluidSim& sim);

void PrintDiagonal(const glm::ivec2& size, Vortex2D::Renderer::Buffer<float>& buffer);
void PrintWeights(const glm::ivec2& size, FluidSim& sim);

void PrintVelocity(const Vortex2D::Renderer::Device& device, const glm::ivec2& size, Vortex2D::Fluid::Velocity& velocity);

void PrintVelocity(const glm::ivec2& size, FluidSim& sim);

void CheckVelocity(const Vortex2D::Renderer::Device& device,
                          const glm::ivec2& size,
                          Vortex2D::Fluid::Velocity& velocity,
                          FluidSim& sim,
                          float error = 1e-6f);

void CheckValid(const glm::ivec2& size, FluidSim& sim, Vortex2D::Renderer::Buffer<glm::ivec2>& valid);

void CheckDiv(const glm::ivec2& size, Vortex2D::Renderer::Buffer<float>& buffer, FluidSim& sim, float error = 1e-6f);
void PrintDiv(const glm::ivec2& size, FluidSim& sim);
