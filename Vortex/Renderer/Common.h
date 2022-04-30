//
//  Common.h
//  Vortex
//

#pragma once

#include <Vortex/Renderer/Gpu.h>

#include <memory>
#include <vector>

#include <glm/mat2x2.hpp>
#include <glm/mat4x4.hpp>
#include <glm/vec2.hpp>
#include <glm/vec4.hpp>

#ifdef _WIN32
#ifdef VORTEX2D_API_EXPORTS
#define VORTEX_API __declspec(dllexport)
#else
#define VORTEX_API __declspec(dllimport)
#endif
#else
#define VORTEX_API
#endif

namespace Vortex
{
namespace Renderer
{
/**
 * @brief A binary SPIRV shader, to be feed to vulkan.
 */
class SpirvBinary
{
public:
  template <std::size_t N>
  SpirvBinary(const uint32_t (&spirv)[N]) : mData(spirv), mSize(N * 4)
  {
  }

  const uint32_t* data() const { return mData; }

  std::size_t size() const { return mSize; }

  std::size_t words() const { return mSize / 4; }

private:
  const uint32_t* mData;
  std::size_t mSize;
};

}  // namespace Renderer
}  // namespace Vortex
