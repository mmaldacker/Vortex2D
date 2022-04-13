//
//  Instance.h
//  Vortex
//

#pragma once

#include <Vortex/Renderer/Common.h>
#include <Vortex/Renderer/Vulkan/Vulkan.h>

#include <memory>
#include <string>
#include <vector>

#define VK_LAYER_LUNARG_STANDARD_VALIDATION_NAME "VK_LAYER_LUNARG_standard_validation"

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
  VORTEX_API Instance(const std::string& name,
                      std::vector<const char*> extensions,
                      bool validation);
  VORTEX_API ~Instance();

  VORTEX_API vk::PhysicalDevice GetPhysicalDevice() const;
  VORTEX_API vk::Instance GetInstance() const;

private:
  vk::UniqueInstance mInstance;
  vk::DispatchLoaderDynamic mLoader;
  vk::PhysicalDevice mPhysicalDevice;
  vk::DebugReportCallbackEXT mDebugCallback;
};

}  // namespace Renderer
}  // namespace Vortex
