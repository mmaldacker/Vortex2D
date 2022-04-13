#include <Vortex/Renderer/Vulkan/Device.h>
#include <Vortex/Renderer/Vulkan/Instance.h>
#include <gtest/gtest.h>

Vortex::Renderer::Device* device;

TEST(Vulkan, Init)
{
  auto physicalDevice = static_cast<Vortex::Renderer::VulkanDevice*>(device)->GetPhysicalDevice();
  auto properties = physicalDevice.getProperties();
  std::cout << "Device name: " << properties.deviceName << std::endl;

  for (int i = 0; i < 3; i++)
  {
    std::cout << "Max local size " << i << " : " << properties.limits.maxComputeWorkGroupSize[i]
              << std::endl;
  }
  std::cout << "Max local size total: " << properties.limits.maxComputeWorkGroupInvocations
            << std::endl;
}

int main(int argc, char** argv)
{
#ifdef NDEBUG
  bool debug = false;
#else
  bool debug = true;
#endif

  Vortex::Renderer::Instance instance("Tests", {}, debug);
  Vortex::Renderer::VulkanDevice device_(instance);

  device = &device_;

  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
