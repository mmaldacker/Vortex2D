//
//  GLFW.cpp
//  Vortex2D
//

#include "glfw.h"

#include <stdexcept>
#include <string>
#include <iostream>

static void ErrorCallback(int error, const char* description)
{
    throw std::runtime_error("GLFW Error: " +
                             std::to_string(error) +
                             " What: " +
                             std::string(description));
}

static PFN_vkCreateDebugReportCallbackEXT pfn_vkCreateDebugReportCallbackEXT;
VkResult vkCreateDebugReportCallbackEXT(VkInstance instance,
                                        const VkDebugReportCallbackCreateInfoEXT* pCreateInfo,
                                        const VkAllocationCallbacks* pAllocator,
                                        VkDebugReportCallbackEXT* pCallback)
{
    return pfn_vkCreateDebugReportCallbackEXT(instance, pCreateInfo, pAllocator, pCallback);
}

static PFN_vkDestroyDebugReportCallbackEXT pfn_vkDestroyDebugReportCallbackEXT;
void vkDestroyDebugReportCallbackEXT(VkInstance instance,
                                     VkDebugReportCallbackEXT callback,
                                     const VkAllocationCallbacks* pAllocator)
{
    pfn_vkDestroyDebugReportCallbackEXT(instance, callback, pAllocator);
}


VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugReportFlagsEXT flags,
                                             VkDebugReportObjectTypeEXT objType,
                                             uint64_t obj,
                                             size_t location,
                                             int32_t code,
                                             const char* layerPrefix,
                                             const char* msg,
                                             void* userData)
{
    std::cout << "validation layer: " << msg << std::endl;
    return VK_FALSE;
}

GLFWApp::GLFWApp(uint32_t width, uint32_t height, bool visible, bool validation)
    : mWidth(width)
    , mHeight(height)
{
    glfwSetErrorCallback(ErrorCallback);

    if (!glfwInit())
    {
        throw std::runtime_error("Could not initialise GLFW!");
    }

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);
    glfwWindowHint(GLFW_VISIBLE, visible ? GLFW_TRUE : GLFW_FALSE);

    mWindow = glfwCreateWindow(width, height, "Vortex2D App", nullptr, nullptr);
    if (!mWindow)
    {
        throw std::runtime_error("Error creating GLFW Window");
    }

    std::vector<const char*> extensions;
    unsigned int glfwExtensionCount = 0;
    const char** glfwExtensions;

    // get the required extensions from GLFW
    glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
    for (int i = 0; i < glfwExtensionCount; i++)
    {
        extensions.push_back(glfwExtensions[i]);
    }

    // add the validation extension if necessary
    if (validation)
    {
        extensions.push_back(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);
    }

    // configure instance
    vk::ApplicationInfo appInfo;
    appInfo.setPApplicationName("Vortex2D App");

    const std::vector<const char*> validationLayers = {"VK_LAYER_LUNARG_standard_validation"};

    vk::InstanceCreateInfo instanceInfo;
    instanceInfo
            .setPApplicationInfo(&appInfo)
            .setEnabledExtensionCount(extensions.size())
            .setPpEnabledExtensionNames(extensions.data());

    // add the validation layer if necessary
    if (validation)
    {
        instanceInfo.setEnabledLayerCount(validationLayers.size());
        instanceInfo.setPpEnabledLayerNames(validationLayers.data());
    }

    mInstance = vk::createInstanceUnique(instanceInfo);

    // add the validation calback if necessary
    if (validation)
    {
        pfn_vkCreateDebugReportCallbackEXT = (PFN_vkCreateDebugReportCallbackEXT) mInstance->getProcAddr("vkCreateDebugReportCallbackEXT");
        pfn_vkDestroyDebugReportCallbackEXT = (PFN_vkDestroyDebugReportCallbackEXT) mInstance->getProcAddr("vkDestroyDebugReportCallbackEXT");

        vk::DebugReportCallbackCreateInfoEXT debugCallbackInfo;
        debugCallbackInfo
                .setPfnCallback(debugCallback)
                .setFlags(vk::DebugReportFlagBitsEXT::eWarning | vk::DebugReportFlagBitsEXT::eError);

        mDebugCallback = mInstance->createDebugReportCallbackEXTUnique(debugCallbackInfo);
    }

    // create surface
    VkSurfaceKHR surface;
    if (glfwCreateWindowSurface(*mInstance, mWindow, nullptr, &surface) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create window surface!");
    }

    mSurface = vk::UniqueSurfaceKHR(surface, vk::SurfaceKHRDeleter(*mInstance));
}

GLFWApp::~GLFWApp()
{
    glfwDestroyWindow(mWindow);
    glfwTerminate();
}

void GLFWApp::Run()
{
    while (!glfwWindowShouldClose(mWindow))
    {
        glfwPollEvents();
    }
}

void GLFWApp::SetKeyCallback(GLFWkeyfun cbfun)
{
    glfwSetKeyCallback(mWindow, cbfun);
}

vk::PhysicalDevice GLFWApp::GetPhysicalDevice() const
{
    // TODO better search than first available device
    // - using swap chain info
    // - using queue info
    // - discrete GPU
    vk::PhysicalDevice physicalDevice = mInstance->enumeratePhysicalDevices().at(0);
    auto properties = physicalDevice.getProperties();
    std::cout << "Device name: " << properties.deviceName << std::endl;

    return physicalDevice;
}

vk::SurfaceKHR GLFWApp::GetSurface() const
{
    return *mSurface;
}
