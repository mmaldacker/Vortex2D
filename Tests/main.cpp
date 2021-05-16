#include <Vortex2D/Renderer/Device.h>
#include <Vortex2D/Renderer/Instance.h>
#include <gtest/gtest.h>

Vortex::Renderer::Device* device;

TEST(Vulkan, Init)
{
  auto physicalDevice = device->GetPhysicalDevice();
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
  Vortex::Renderer::Device device_(instance);

  device = &device_;

  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
