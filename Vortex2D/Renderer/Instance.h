//
//  Instance.h
//  Vortex2D
//

#ifndef Instance_h
#define Instance_h

#include <Vortex2D/Renderer/Common.h>

#include <string>
#include <vector>

namespace Vortex2D { namespace Renderer {

class Instance
{
public:
    void Create(const std::string& name, std::vector<const char*> extensions, bool validation);

    vk::PhysicalDevice GetPhysicalDevice() const;
    vk::Instance GetInstance() const;

private:
    vk::UniqueInstance mInstance;
    vk::PhysicalDevice mPhysicalDevice;
    vk::UniqueDebugReportCallbackEXT mDebugCallback;
};

void RemoveInexistingExtensions(std::vector<const char*>& list, const std::vector<vk::ExtensionProperties>& available);
void RemoveInexistingLayers(std::vector<const char*>& list, const std::vector<vk::LayerProperties>& available);

}}

#endif
