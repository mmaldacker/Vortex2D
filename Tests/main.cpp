#ifdef VORTEX2D_BACKEND_VULKAN
#include <Vortex/Renderer/Vulkan/Device.h>
#include <Vortex/Renderer/Vulkan/Instance.h>
#else
#include <Vortex/Renderer/WebGPU/Device.h>
#include <Vortex/Renderer/WebGPU/Instance.h>
#endif
#include <gtest/gtest.h>

Vortex::Renderer::Device* device;

int main(int argc, char** argv)
{
#ifdef NDEBUG
  bool debug = false;
#else
  bool debug = true;
#endif

#ifdef VORTEX2D_BACKEND_VULKAN
  Vortex::Renderer::Instance instance("Tests", {}, debug);
  Vortex::Renderer::VulkanDevice device_(instance);
#else
  Vortex::Renderer::Instance instance;
  Vortex::Renderer::WebGPUDevice device_(instance);
#endif

  device = &device_;

  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
