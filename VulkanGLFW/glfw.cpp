//
//  GLFW.cpp
//  Vortex2D
//

#include "glfw.h"

#include <stdexcept>
#include <string>
#include <iostream>

void ErrorCallback(int error, const char* description)
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
    std::cerr << "validation layer: " << msg << std::endl;
    return VK_FALSE;
}

GLFWApp::GLFWApp(int width, int height, bool visible, bool validation)
{
    glfwSetErrorCallback(ErrorCallback);

    if (!glfwInit())
    {
        throw std::runtime_error("Could not initialise GLFW!");
    }

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

    vk::InstanceCreateInfo instanceInfo;
    instanceInfo.setPApplicationInfo(&appInfo);
    instanceInfo.setEnabledExtensionCount(extensions.size());
    instanceInfo.setPpEnabledExtensionNames(extensions.data());

    // add the validation layer if necessary
    const std::vector<const char*> validationLayers = {"VK_LAYER_LUNARG_standard_validation"};
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
        debugCallbackInfo.setPfnCallback(debugCallback);

        mDebugCallback = mInstance->createDebugReportCallbackEXTUnique(debugCallbackInfo);
    }
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
