//
//  Instance.h
//  Vortex2D
//

#ifndef Instance_h
#define Instance_h

#include <Vortex2D/Renderer/Common.h>

#include <string>
#include <vector>

#define VK_LAYER_LUNARG_STANDARD_VALIDATION_NAME "VK_LAYER_LUNARG_standard_validation"

namespace Vortex2D { namespace Renderer {

class Instance
{
public:
    Instance(const std::string& name, std::vector<const char*> extensions, bool validation);

    vk::PhysicalDevice GetPhysicalDevice() const;
    vk::Instance GetInstance() const;

private:
    vk::UniqueInstance mInstance;
    vk::PhysicalDevice mPhysicalDevice;
    vk::UniqueDebugReportCallbackEXT mDebugCallback;
};

bool HasLayer(const char* extension, const std::vector<vk::LayerProperties>& availableExtensions);
bool HasExtension(const char* extension, const std::vector<vk::ExtensionProperties>& availableExtensions);

}}

#endif
