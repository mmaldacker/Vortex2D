//
//  Common.h
//  Vortex
//

#ifndef Vortex2D_Common_h
#define Vortex2D_Common_h

#define VULKAN_HPP_TYPESAFE_CONVERSION 1
#include <vulkan/vulkan.hpp>

#define VMA_RECORDING_ENABLED 0

#include <glm/mat2x2.hpp>
#include <glm/mat4x4.hpp>
#include <glm/vec2.hpp>
#include <glm/vec4.hpp>

#ifdef _WIN32
#ifdef VORTEX2D_API_EXPORTS
#define VORTEX2D_API __declspec(dllexport)
#else
#define VORTEX2D_API __declspec(dllimport)
#endif
#else
#define VORTEX2D_API
#endif

#endif
