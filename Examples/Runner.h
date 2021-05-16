//
//  Runner.h
//  Vortex
//

#pragma once

#include <Vortex2D/Renderer/Device.h>
#include <Vortex2D/Renderer/RenderTarget.h>

class Runner
{
public:
  virtual ~Runner() {}
  virtual void Init(const Vortex::Renderer::Device& device,
                    Vortex::Renderer::RenderTarget& renderTarget) = 0;
  virtual void Step() = 0;
};
