//
//  Instance.h
//  Vortex2D
//

#ifndef Vortex2d_Instance_h
#define Vortex2d_Instance_h

#include <Vortex2D/Renderer/Common.h>

#include <string>
#include <vector>

#define VK_LAYER_LUNARG_STANDARD_VALIDATION_NAME "VK_LAYER_LUNARG_standard_validation"

namespace Vortex2D
{
namespace Renderer
{
/**
 * @brief Vulkan instance, which extensions enabled.
 */
class Instance
{
public:
  VORTEX2D_API Instance(const std::string& name,
                        std::vector<const char*> extensions,
                        bool validation);
  VORTEX2D_API ~Instance();

  VORTEX2D_API vk::PhysicalDevice GetPhysicalDevice() const;
  VORTEX2D_API vk::Instance GetInstance() const;

private:
  vk::UniqueInstance mInstance;
  vk::DispatchLoaderDynamic mLoader;
  vk::PhysicalDevice mPhysicalDevice;
  vk::DebugReportCallbackEXT mDebugCallback;
};

bool HasLayer(const char* extension, const std::vector<vk::LayerProperties>& availableExtensions);
bool HasExtension(const char* extension,
                  const std::vector<vk::ExtensionProperties>& availableExtensions);

}  // namespace Renderer
}  // namespace Vortex2D

#endif
