//
//  Runner.h
//  Vortex
//

#pragma once

#include <Vortex/Renderer/Device.h>
#include <Vortex/Renderer/RenderTarget.h>

class Runner
{
public:
  virtual ~Runner() {}
  virtual void Init(Vortex::Renderer::Device& device,
                    Vortex::Renderer::RenderTarget& renderTarget) = 0;
  virtual void Step() = 0;
};
