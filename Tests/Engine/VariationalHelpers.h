//
//  Common.h
//  Vortex
//

#pragma once

#include <iostream>
#include <random>
#include <vector>

#include <gmock/gmock.h>

#include <Vortex/Renderer/Buffer.h>
#include <Vortex/Renderer/CommandBuffer.h>
#include <Vortex/Renderer/Texture.h>

#include <Vortex/Engine/LinearSolver/LinearSolver.h>
#include <Vortex/Engine/Velocity.h>

#include "fluidsim.h"

// Boundary definition - several circles in a circular domain.

const Vec2f c0(0.5f, 0.5f), c1(0.7f, 0.5f), c2(0.3f, 0.35f), c3(0.5f, 0.7f);
const float rad0 = 0.4f, rad1 = 0.1f, rad2 = 0.1f, rad3 = 0.1f;

float circle_phi(const Vec2f& position, const Vec2f& centre, float radius);

float boundary_phi(const Vec2f& position);

float complex_boundary_phi(const Vec2f& position);

void AddParticles(const glm::ivec2& size, FluidSim& sim, float (*phi)(const Vec2f&));
void SetVelocity(Vortex::Renderer::Device& device,
                 const glm::ivec2& size,
                 Vortex::Fluid::Velocity& velocity,
                 FluidSim& sim);

void SetSolidPhi(Vortex::Renderer::Device& device,
                 const glm::ivec2& size,
                 Vortex::Renderer::Texture& solidPhi,
                 FluidSim& sim,
                 float scale = 1.0f);

void SetLiquidPhi(Vortex::Renderer::Device& device,
                  const glm::ivec2& size,
                  Vortex::Renderer::Texture& liquidPhi,
                  FluidSim& sim,
                  float scale = 1.0f);

void BuildInputs(Vortex::Renderer::Device& device,
                 const glm::ivec2& size,
                 FluidSim& sim,
                 Vortex::Fluid::Velocity& velocity,
                 Vortex::Renderer::Texture& solidPhi,
                 Vortex::Renderer::Texture& liquidPhi);

void BuildLinearEquation(const glm::ivec2& size,
                         Vortex::Renderer::Buffer<float>& d,
                         Vortex::Renderer::Buffer<glm::vec2>& l,
                         Vortex::Renderer::Buffer<float>& div,
                         FluidSim& sim);

void PrintDiagonal(const glm::ivec2& size, Vortex::Renderer::Buffer<float>& buffer);
void PrintWeights(const glm::ivec2& size, FluidSim& sim);

void PrintVelocity(Vortex::Renderer::Device& device,
                   const glm::ivec2& size,
                   Vortex::Renderer::Texture& velocity);

void PrintVelocity(const glm::ivec2& size, FluidSim& sim);

void CheckVelocity(Vortex::Renderer::Device& device,
                   const glm::ivec2& size,
                   Vortex::Fluid::Velocity& velocity,
                   FluidSim& sim,
                   float error = 1e-6f);

void CheckVelocity(Vortex::Renderer::Device& device,
                   const glm::ivec2& size,
                   Vortex::Renderer::Texture& velocity,
                   const std::vector<glm::vec2>& velocityData,
                   float error = 1e-6f);

void CheckValid(const glm::ivec2& size, FluidSim& sim, Vortex::Renderer::Buffer<glm::ivec2>& valid);

void CheckDiv(const glm::ivec2& size,
              Vortex::Renderer::Buffer<float>& buffer,
              FluidSim& sim,
              float error = 1e-6f);
void PrintDiv(const glm::ivec2& size, FluidSim& sim);

void CheckPressure(const glm::ivec2& size,
                   const std::vector<double>& pressure,
                   Vortex::Renderer::Buffer<float>& bufferPressure,
                   float error);
