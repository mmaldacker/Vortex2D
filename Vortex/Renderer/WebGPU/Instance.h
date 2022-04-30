//
//  Instance.h
//  Vortex2D
//

#ifndef Vortex2d_Vulkan_Instance_h
#define Vortex2d_Vulkan_Instance_h

#include <Vortex/Renderer/Common.h>

#include "WebGPU.h"

#include <future>
#include <memory>
#include <string>
#include <vector>

namespace Vortex
{
namespace Renderer
{
/**
 * @brief Vulkan instance, which extensions enabled.
 */
class Instance
{
public:
  VORTEX_API Instance();
  VORTEX_API ~Instance();

  WGPUAdapter GetAdapter() const;

private:
  WGPUInstance mInstance;
  WGPUAdapter mAdapter;
};

}  // namespace Renderer
}  // namespace Vortex

#endif
