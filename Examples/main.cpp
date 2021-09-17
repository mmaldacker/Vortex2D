#include <Vortex/Vortex.h>

#include <GLFW/glfw3.h>

#include "BuoyancyWaterExample.h"
#include "ObstacleSmokeExample.h"
#include "RenderExample.h"
#include "SmokeExample.h"
#include "SmokeVelocityExample.h"
#include "WaterExample.h"
#include "WaterFallExample.h"
#include "WatermillExample.h"

#include <chrono>
#include <functional>
#include <iostream>
#include <memory>
#include <numeric>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/transform.hpp>

using namespace Vortex;

glm::vec4 green = glm::vec4(92.0f, 173.0f, 159.0f, 255.0f) / glm::vec4(255.0f);
glm::vec4 gray = glm::vec4(80.0f, 81.0f, 79.0f, 255.0f) / glm::vec4(255.0f);
glm::vec4 blue = glm::vec4(36.0f, 123.0f, 160.0f, 255.0f) / glm::vec4(255.0f);

static void ErrorCallback(int error, const char* description)
{
  throw std::runtime_error("GLFW Error: " + std::to_string(error) +
                           " What: " + std::string(description));
}

std::vector<const char*> GetGLFWExtensions()
{
  std::vector<const char*> extensions;
  unsigned int glfwExtensionCount = 0;
  const char** glfwExtensions;

  // get the required extensions from GLFW
  glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
  for (unsigned int i = 0; i < glfwExtensionCount; i++)
  {
    extensions.push_back(glfwExtensions[i]);
  }

  return extensions;
}

vk::UniqueSurfaceKHR GetGLFWSurface(GLFWwindow* window, vk::Instance instance)
{
  // create surface
  VkSurfaceKHR surface;
  if (glfwCreateWindowSurface(static_cast<VkInstance>(instance), window, nullptr, &surface) !=
      VK_SUCCESS)
  {
    throw std::runtime_error("failed to create window surface!");
  }

  vk::UniqueSurfaceKHR uniqueSurface(surface, instance);
  return uniqueSurface;
}

GLFWwindow* GetGLFWWindow(const glm::ivec2& size)
{
  GLFWwindow* window = glfwCreateWindow(size.x, size.y, "Vortex2D App", nullptr, nullptr);
  if (window == nullptr)
  {
    throw std::runtime_error("Could not create glfw window");
  }

  return window;
}

class App
{
public:
  glm::ivec2 size = {256, 256};
  glm::ivec2 windowSize = {512, 512};
  float windowScale = 2.0f;
  float delta = 0.016f;

  App(bool validation = true)
      : glfwWindow(GetGLFWWindow(windowSize))
      , instance("Vortex2D", GetGLFWExtensions(), validation)
      , surface(GetGLFWSurface(glfwWindow, static_cast<VkInstance>(instance.GetInstance())))
      , device(instance, *surface, validation)
      , window(device, *surface, (uint32_t)(windowSize.x), (uint32_t)(windowSize.y))
      , clearRender(
            window.Record({std::make_shared<Renderer::Clear>(glm::vec4{0.1f, 0.1f, 0.1f, 1.0f})}))
  {
    // setup callback
    glfwSetWindowUserPointer(glfwWindow, this);
    auto func = [](GLFWwindow* window, int key, int scancode, int action, int mods)
    { static_cast<App*>(glfwGetWindowUserPointer(window))->KeyCallback(key, action); };
    glfwSetKeyCallback(glfwWindow, func);

    window.View = glm::scale(glm::vec3{windowScale, windowScale, 1.0f});

    // Set default scene
    example.reset(new WaterExample(device, size, delta));
    example->Init(device, window);
  }

  ~App()
  {
    device.Handle().waitIdle();
    glfwDestroyWindow(glfwWindow);
  }

  void KeyCallback(int key, int action)
  {
    if (action != GLFW_PRESS)
      return;

    device.Handle().waitIdle();

    switch (key)
    {
      case GLFW_KEY_1:
        example.reset(new RenderExample(device, size));
        break;
      case GLFW_KEY_2:
        example.reset(new SmokeExample(device, size, delta));
        break;
      case GLFW_KEY_3:
        example.reset(new ObstacleSmokeExample(device, size, delta));
        break;
      case GLFW_KEY_4:
        example.reset(new SmokeVelocityExample(device, size, delta));
        break;
      case GLFW_KEY_5:
        example.reset(new WaterExample(device, size, delta));
        break;
      case GLFW_KEY_6:
        example.reset(new WaterFallExample(device, size, delta));
        break;
      case GLFW_KEY_7:
        example.reset(new WatermillExample(device, size, delta));
        break;
      case GLFW_KEY_8:
        example.reset(new HydrostaticWaterExample(device, size, delta));
        break;
      default:
        return;
    }

    example->Init(device, window);
  }

  void Run()
  {
    const uint64_t windowSize = 20;
    std::vector<uint64_t> timePoints(windowSize, 0);
    int timePointIndex = 0;

    while (!glfwWindowShouldClose(glfwWindow))
    {
      glfwPollEvents();

      auto start = std::chrono::system_clock::now();

      clearRender.Submit();
      if (example)
        example->Step();
      window.Display();

      auto end = std::chrono::system_clock::now();
      timePoints[timePointIndex] =
          std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
      timePointIndex = (timePointIndex + 1) % windowSize;

      auto average = std::accumulate(timePoints.begin(), timePoints.end(), 0ll) / windowSize;
      auto title = std::to_string(average) + "ms";
      glfwSetWindowTitle(glfwWindow, title.c_str());
    }
  }

  GLFWwindow* glfwWindow;
  Vortex::Renderer::Instance instance;
  vk::UniqueSurfaceKHR surface;
  Vortex::Renderer::Device device;
  Renderer::RenderWindow window;
  std::unique_ptr<Runner> example;
  Vortex::Renderer::RenderCommand clearRender;
};

int main()
{
  if (!glfwInit())
  {
    throw std::runtime_error("Could not initialise GLFW!");
  }

  try
  {
    glfwSetErrorCallback(ErrorCallback);
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);
    glfwWindowHint(GLFW_COCOA_RETINA_FRAMEBUFFER, GL_FALSE);

#ifdef NDEBUG
    bool debug = false;
#else
    bool debug = true;
#endif

    App app(debug);
    app.Run();
  }
  catch (const std::exception& error)
  {
    std::cout << "exception: " << error.what() << std::endl;
  }

  glfwTerminate();
}
